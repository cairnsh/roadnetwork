/* A function that takes a line segment and a list of line segments, and finds the
 * closest one in the list. */
#include "Python.h"
#include "stdlib.h"

struct line {
    double x1, y1, x2, y2;
};

struct point {
    double x, y;
};

#define TYPEERROR(err) PyErr_SetString(PyExc_TypeError, err)
//#define DEBUG

int pylist_to_line(PyObject *curlinept, struct line *curline)
{
    PyObject *tmp;
    // We know it's a list
    if(PyList_Size(curlinept) != 4) {
        if(!PyErr_Occurred())
            TYPEERROR("Invalid line in list: length is not 4.");
        goto error;
    }
#define GETITEM_LIST(xx, index) tmp = PyList_GetItem(curlinept, index);\
    if(PyErr_Occurred()) goto error;\
    curline->xx = PyFloat_AsDouble(tmp);\
    if(PyErr_Occurred()) goto error;

    GETITEM_LIST(x1, 0);
    GETITEM_LIST(y1, 1);
    GETITEM_LIST(x2, 2);
    GETITEM_LIST(y2, 3);

    return 1;
error: // already exception
    return 0;
}

int pytuple_to_line(PyObject *curlinept, struct line *curline)
{
    PyObject *tmp;
    // We know it's a tuple
    if(PyTuple_Size(curlinept) != 4) {
        if(!PyErr_Occurred())
            TYPEERROR("Invalid line in tuple: length is not 4.");
        goto error;
    }
#define GETITEM_TUPLE(xx, index) tmp = PyTuple_GetItem(curlinept, index);\
    if(PyErr_Occurred()) goto error;\
    curline->xx = PyFloat_AsDouble(tmp);\
    if(PyErr_Occurred()) goto error;

    GETITEM_TUPLE(x1, 0);
    GETITEM_TUPLE(y1, 1);
    GETITEM_TUPLE(x2, 2);
    GETITEM_TUPLE(y2, 3);

    return 1;
error: // already exception
    return 0;
}

int pyobject_to_line(PyObject *curlinept, struct line *curline)
{
    if(PyList_Check(curlinept)) {
        return pylist_to_line(curlinept, curline);
            return 0;
        return 1;
    }
    else if(PyTuple_Check(curlinept)) {
        return pytuple_to_line(curlinept, curline);
    }
    else {
        PyErr_SetString(PyExc_TypeError, "Non-tuple or list in line segment list.");
        return 0;
    }
}

int pylist_getitem_and_toline(PyObject *list, Py_ssize_t idx, struct line *line)
{
    PyObject *curlinept = PyList_GetItem(list, idx);
    if(PyErr_Occurred() || !pyobject_to_line(curlinept, line))
        return 0;
    return 1;
}

void line_to_pts(struct line *line, struct point *p1, struct point *p2)
{
    p1->x = line->x1;
    p1->y = line->y1;
    p2->x = line->x2;
    p2->y = line->y2;
}

void pts_to_line(struct point *p1, struct point *p2, struct line *line)
{
    line->x1 = p1->x;
    line->y1 = p1->y;
    line->x2 = p2->x;
    line->y2 = p2->y;
}

struct quadratic_problem {
    // (at + bs + e)^2 + (ct + ds + f)^2
    double a, b, c, d, e, f;
};

void lines_to_quadratic(struct line *l1, struct line *l2, struct quadratic_problem *qq)
{
    double a = (l1->x2 - l1->x1);
    double b = -(l2->x2 - l2->x1);
    double e = l1->x1 - l2->x1;
    double c = l1->y2 - l1->y1;
    double d = -(l2->y2 - l2->y1);
    double f = l1->y1 - l2->y1;
    qq->a = a; qq->b = b; qq->c = c; qq->d = d; qq->e = e; qq->f = f;
}

#define TINY 1e-6

double itp(double x, double z, double y)
{
    return (y-x) / (z-x);
}

double sqdist(double a, double b) { return a*a + b*b; }

double minimize_1d_quadratic_problem(double a, double b, double e, double f, double *T)
{
    *T = -(a*e + b*f) / (a*a + b*b);
    return sqdist(*T*a + e, *T*b + f);
}

double evaluate_quadratic_problem_at(struct quadratic_problem *q, double t, double s)
{
    return sqdist(q->a*t + q->b*s + q->e, q->c*t + q->d*s + q->f);
}

int solve_unconstrained_quadratic_problem(struct quadratic_problem *q, double *t, double *s,
    double *sqdistance)
{
    double a = q->a,
           b = q->b,
           c = q->c,
           d = q->d,
           e = q->e,
           f = q->f;
    double det = a*d - b*c;
    if(det * det < TINY * (a*a+b*b)*(c*c+d*d)) {
        // determinant almost zero
        // so we are really just minimizing (x + e)^2 + (hx + f)^2 where x = at + bs
        double scale = fabs(b) > fabs(a) ? d/b : c/a;
#ifdef DEBUG
        printf("The problem is: (%f t + %f s + %f)**2 + (%f t + %f s + %f)**2\n", a, b, e, c, d, f);
#endif

        /* We are minimizing a 1d quadratic problem on
         * all four lines (i.e. intervals) in the square.
         * If an interval contains the unique minimum,
         * then we're done. If not, then we return the
         * minimum on corners of the square. I don't
         * know if there is a smarter way to do this.
         */

        // look at all four lines
        double x;
        double T, S;
        double minimum = -1;
        double current;
        minimize_1d_quadratic_problem(1, scale, e, f, &x);

/* If the argument is between zero and one, set *t and *s and return.
 * We don't need to check any more because a minimum of the 1d problem
 * is automatically the smallest. */
#define IF_SENSIBLE_RETURN(xx) if(xx >= 0 && xx <= 1) { *t = T; *s = S; return 0; }

#ifdef DEBUG
        printf("The minimum is at %f t + %f s = %f\n", a, b, x);
#endif

        T = itp(0, a, x);
        S = 0;
#ifdef DEBUG
        printf("first try: T=%f S=%f\n", T, S);
#endif
        IF_SENSIBLE_RETURN(T)

        T = 1;
        S = itp(a, a+b, x);
#ifdef DEBUG
        printf("second try: T=%f S=%f\n", T, S);
#endif
        IF_SENSIBLE_RETURN(S)

        T = itp(b, a+b, x);
        S = 1;
#ifdef DEBUG
        printf("third try: T=%f S=%f\n", T, S);
#endif
        IF_SENSIBLE_RETURN(T)

        T = 0;
        S = itp(0, b, x);
#ifdef DEBUG
        printf("fourth try: T=%f S=%f\n", T, S);
#endif
        IF_SENSIBLE_RETURN(S)

/* Evaluate the quadratic problem at (T, S) and update the minimum. */
#define EVAL_AND_UPDATE_MIN(T, S) current = evaluate_quadratic_problem_at(q, T, S); \
    if(minimum < 0 || current < minimum) { minimum = current; *t = T; *s = S; }

        // that didn't work, so let's check the four corners
        EVAL_AND_UPDATE_MIN(0, 0)
#ifdef DEBUG
        printf("fifth try: T=%f S=%f\n", T, S);
#endif
        EVAL_AND_UPDATE_MIN(0, 1)
#ifdef DEBUG
        printf("sixth try: T=%f S=%f\n", T, S);
#endif
        EVAL_AND_UPDATE_MIN(1, 1)
#ifdef DEBUG
        printf("seventh try: T=%f S=%f\n", T, S);
#endif
        EVAL_AND_UPDATE_MIN(1, 0)
#ifdef DEBUG
        printf("eighth try: T=%f S=%f\n", T, S);
#endif
        return 0;
    }
    else {
        /* solve
         * at + bs + e = 0
         * ct + ds + f = 0
         */
        *t = (-e*d + f*b) / det;
        *s = (+e*c - f*a) / det;
        return 1;
    }
}

void solve_quadratic_problem(struct quadratic_problem *q, double *t, double *s, double *sqdistance)
{
    double T, S, current;
    // minimize quadratic_problem in the unit square [0, 1] x [0, 1]
    // there are nine possibilities:
    // the minimum may be at the intersection of the two lines
    if(!solve_unconstrained_quadratic_problem(q, t, s, sqdistance)) {
        //printf("Determinant zero situation done\n");
        return;
    }
#ifdef DEBUG
    printf("Intersection coordinate: %f %f\n", *t, *s);
#endif
    if(*t >= 0 && *t <= 1 && *s >= 0 && *s <= 1) {
        // the body minimum is inside the unit square
        // (the lines intersect & the distance is zero)
        *sqdistance = 0;
#ifdef DEBUG
        printf("Intersection inside square\n");
#endif
        return;
    }

    //printf("Looking at edges\n");

    // it may be on one of the four edges

#ifdef DEBUG
    printf("Quadratic problem: (%f t + %f s + %f)**2 + (%f t + %f s + %f)**2\n", q->a, q->b, q->e, q->c, q->d, q->f);
#endif

#define UPDATEMIN { if(*sqdistance < 0 || current < *sqdistance) { *t = T; *s = S; *sqdistance = current; } }
    *sqdistance = -1;

    S = 0;
    current = minimize_1d_quadratic_problem(q->a, q->c, q->e, q->f, &T);
    if(T >= 0 && T <= 1)
        UPDATEMIN
#ifdef DEBUG
    printf("Current min: %f %f %f (current=%f)\n", *t, *s, *sqdistance, current);
#endif

    T = 1;
    current = minimize_1d_quadratic_problem(q->b, q->d, q->a+q->e, q->c+q->f, &S);
    if(S >= 0 && S <= 1)
        UPDATEMIN
#ifdef DEBUG
    printf("Current min: %f %f %f (current=%f)\n", *t, *s, *sqdistance, current);
#endif

    S = 1;
    current = minimize_1d_quadratic_problem(q->a, q->c, q->b+q->e, q->d+q->f, &T);
    if(T >= 0 && T <= 1)
        UPDATEMIN
#ifdef DEBUG
    printf("Current min: %f %f %f (current=%f)\n", *t, *s, *sqdistance, current);
#endif

    T = 0;
    current = minimize_1d_quadratic_problem(q->b, q->d, q->e, q->f, &S);
    if(S >= 0 && S <= 1)
        UPDATEMIN
#ifdef DEBUG
    printf("Current min: %f %f %f (current=%f)\n", *t, *s, *sqdistance, current);
#endif

    // or one of the four vertices

    S = 0; T = 0;
    current = sqdist(q->e, q->f);
    UPDATEMIN
#ifdef DEBUG
    printf("Current min: %f %f %f (current=%f)\n", *t, *s, *sqdistance, current);
#endif

    S = 0; T = 1;
    current = sqdist(q->a + q->e, q->c + q->f);
    UPDATEMIN
#ifdef DEBUG
    printf("Current min: %f %f %f (current=%f)\n", *t, *s, *sqdistance, current);
#endif

    S = 1; T = 1;
    current = sqdist(q->a + q->b + q->e, q->c + q->d + q->f);
    UPDATEMIN
#ifdef DEBUG
    printf("Current min: %f %f %f (current=%f)\n", *t, *s, *sqdistance, current);
#endif

    S = 1; T = 0;
    current = sqdist(q->b + q->e, q->d + q->f);
    UPDATEMIN
#ifdef DEBUG
    printf("Current min: %f %f %f (current=%f)\n", *t, *s, *sqdistance, current);
#endif

    return;
}

void get_point_on_line(struct line *l1, double t, struct point *p)
{
    p->x = (1-t) * l1->x1 + t * l1->x2;
    p->y = (1-t) * l1->y1 + t * l1->y2;
}

double closest_line_between_lines(struct line *l1, struct line *l2, struct line *l0)
{
    struct quadratic_problem qq;
    double t, s, sqdistance;
    struct point p1, p2;
    lines_to_quadratic(l1, l2, &qq);
    solve_quadratic_problem(&qq, &t, &s, &sqdistance);
#ifdef DEBUG
    printf("t=%f s=%f dist=%f\n", t, s, sqrt(sqdistance));
#endif
    get_point_on_line(l1, t, &p1);
    get_point_on_line(l2, s, &p2);
    pts_to_line(&p1, &p2, l0);
    return sqdistance;
}

double closest_line_between_a_pt_and_a_line(struct point *p, struct line *l, struct line *o)
{
    // 1d quadratic problem: (at + e)**2 + (bt + f)**2
    double a = l->x2 - l->x1;
    double b = l->y2 - l->y1;
    double e = l->x1 - p->x;
    double f = l->y1 - p->y;

    double minimum;
    double t;
    struct point x;

    minimum = minimize_1d_quadratic_problem(a, b, e, f, &t); // unique
#ifdef DEBUG
    printf("(%f t + %f)**2 + (%f t + %f)**2 minimized at t=%f (%f)\n", a, e, b, f, t, minimum);
#endif
    if(t < 0 || t > 1) {
        /* We only need to check the corners if
         * the minimum is outside the interval.
         */
        double dist_at_0 = sqdist(e, f),
               dist_at_1 = sqdist(a+e, b+f);
        if(dist_at_1 < dist_at_0) {
            t = 1;
            minimum = dist_at_1;
        }
        else {
            t = 0;
            minimum = dist_at_0;
        }
    }
    get_point_on_line(l, t, &x);
    pts_to_line(p, &x, o);
    return minimum;
}

static PyObject *
shortest_line(PyObject *self, PyObject *args)
{
    struct line l1, l2;
    struct line closest;
    if(!PyArg_ParseTuple(args, "(dddd)(dddd)", &l1.x1, &l1.y1, &l1.x2, &l1.y2,
                                               &l2.x1, &l2.y1, &l2.x2, &l2.y2))
        return NULL;
    closest_line_between_lines(&l1, &l2, &closest);
    return Py_BuildValue("dddd", closest.x1, closest.y1, closest.x2, closest.y2);
}

static PyObject *
shortest_line_to_array(PyObject *self, PyObject *args)
{
    struct line li;
    struct line best;
    double bestlen = -1;
    PyObject *list;
    Py_ssize_t bestidx = -1;
    Py_ssize_t length;
    Py_ssize_t i;
    if(!PyArg_ParseTuple(args, "(dddd)O", &li.x1, &li.y1, &li.x2, &li.y2, &list))
        return NULL;
    if(!PyList_Check(list)) {
        PyErr_SetString(PyExc_TypeError, "Second argument is not a list.");
        return NULL;
    }
    length = PyList_Size(list);
    for(i = 0; i < length; i++) {
        struct line curline;
        struct line result;
        double curlen;
        if(!pylist_getitem_and_toline(list, i, &curline))
            return NULL;
        curlen = closest_line_between_lines(&li, &curline, &result);
        if(bestlen < 0 || bestlen > curlen) {
            best = result;
            bestlen = curlen;
        }
    }
    if (bestlen == -1)
        return Py_None;
    else
        return Py_BuildValue("dddd", best.x1, best.y1, best.x2, best.y2);
}

static PyObject *
shortest_pt(PyObject *self, PyObject *args)
{
    struct point p;
    struct line result;
    struct line li;
    if(!PyArg_ParseTuple(args, "(dd)(dddd)", &p.x, &p.y, &li.x1, &li.y1, &li.x2, &li.y2))
        return NULL;
    closest_line_between_a_pt_and_a_line(&p, &li, &result);
    return Py_BuildValue("dddd", result.x1, result.y1, result.x2, result.y2);
}

static PyObject *
shortest_pt_to_array(PyObject *self, PyObject *args)
{
    struct point p;
    PyObject *list, *point;
    struct line best;
    double bestlen = -1;
    Py_ssize_t bestidx = -1;
    Py_ssize_t length;
    Py_ssize_t i;
    if(!PyArg_ParseTuple(args, "OO", &point, &list))
        return NULL;
    if(!PyArg_ParseTuple(point, "dd", &p.x, &p.y))
        return NULL;
    if(!PyList_Check(list)) {
        PyErr_SetString(PyExc_TypeError, "Second argument is not a list.");
        return NULL;
    }
    length = PyList_Size(list);
    for(i = 0; i < length; i++) {
        struct line curline;
        struct line result;
        double curlen;
        if(!pylist_getitem_and_toline(list, i, &curline))
            return NULL;
        curlen = closest_line_between_a_pt_and_a_line(&p, &curline, &result);
        if(bestlen < 0 || bestlen > curlen) {
            best = result;
            bestlen = curlen;
        }
    }
    if (bestlen == -1)
        return Py_None;
    else
        return Py_BuildValue("dddd", best.x1, best.y1, best.x2, best.y2);
}

static PyMethodDef closestline_the_methods[] = {
    {"to_line",
    shortest_line,
    METH_VARARGS,
    "to_line(l1, l2). Returns shortest line between two lines"},

    {"to_lines",
    shortest_line_to_array,
    METH_VARARGS,
    "to_lines((x1, y1, x2, y2), list of lines)"},

    {"to_pt",
    shortest_pt,
    METH_VARARGS,
    "to_pt((x1, y1), (x1, y1, x2, y2)"},

    {"pt_to_lines",
    shortest_pt_to_array,
    METH_VARARGS,
    "pt_to_lines(x1, y1, list of lines)"},

    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initshortestline(void) {
    (void) Py_InitModule("shortestline", closestline_the_methods);
}
