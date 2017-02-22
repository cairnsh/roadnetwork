import shortestline
import math, random
import networkx as netx

def sqlength(l):
    if len(l) != 4:
        raise Exception("one weird line %s" % str(l))
    a = l[2] - l[0]
    b = l[3] - l[1]
    return a*a + b*b

class RoadModel:
    def __init__(self):
        self.component_list = {}
        self.componentgraph = netx.Graph()
        self.shortest_lines = {}
        self.next_component_index = 1

        self.componentgraph.add_node("x")
        self.componentgraph.add_node("y")

        self.llengths = open("len.dat", "w")

    def random_point(self):
        x = random.random()
        y = random.random()
        return (x, y)

    def set_edge(self, a, b, lne):
        #print lne
        length = math.sqrt(sqlength(lne))
        self.componentgraph.add_edge(a, b, line=lne, weight=length) # replaces existing edges

    def lower_edge(self, a, b, line):
        x = self.componentgraph[a]
        if b not in x:
            self.set_edge(a, b, line)
        else:
            length = math.sqrt(sqlength(line))
            if x[b]['weight'] > length:
                self.componentgraph.add_edge(a, b, line=line, weight=length)

    def contract_nodes(self, nl):
        node = self.next_component_index
        self.next_component_index += 1
        self.componentgraph.add_node(node)
        self.component_list[node] = []

        nodes_to_be_deleted = {}
        for oldnode in nl:
            nodes_to_be_deleted[oldnode] = 1

        weights = {}

        for oldnode in nl:
            self.component_list[node].extend(self.component_list[oldnode])
            for (destination, attrdict) in self.componentgraph[oldnode].iteritems():
                if destination in nodes_to_be_deleted:
                    "ignore"
                else:
                    "record weight"
                    line, weight = attrdict['line'], attrdict['weight']
                    if destination not in weights or weights[destination]['weight'] > weight:
                        weights[destination] = {'line': line, 'weight': weight}
            self.componentgraph.remove_node(oldnode)
            del self.component_list[oldnode]

        for destination, dir in weights.iteritems():
            line, weight = dir['line'], dir['weight']
            self.componentgraph.add_edge(node, destination, line=line, weight=weight)

        return node

    def add_lines_to_component(self, component, lines):
        for l in lines:
            #print self.component_list
            for ix, line in self.component_list.iteritems():
                if ix != component:
                    connectingline = shortestline.to_lines(l, line)
                    self.lower_edge(component, ix, connectingline)
            self.component_list[component].append(l)

    def add_new_component(self, line):
        node = self.next_component_index
        self.next_component_index += 1
        self.componentgraph.add_node(node)
        self.component_list[node] = []

        self.add_lines_to_component(node, [line])

    def connect(self, p0, p1):
        directline = p0 + p1
        self.set_edge("x", "y", directline)
        for (ix, line) in self.component_list.iteritems():
            self.set_edge("x", ix, shortestline.pt_to_lines(p0, line))
            self.set_edge("y", ix, shortestline.pt_to_lines(p1, line))
        dist,shortest = netx.bidirectional_dijkstra(self.componentgraph, "x", "y")
        self.llengths.write("%f\n" % dist)
        self.llengths.flush()
        #print "shortest = ", shortest
        if len(shortest) == 2:
            "direct connection"
            self.add_new_component(directline)
            return [directline]
        else:
            lines = []
            nodes = []
            for i in range(len(shortest) - 1):
                lines.append(self.componentgraph[shortest[i]][shortest[i+1]]['line'])
                if i != 0:
                    nodes.append(shortest[i])
            node = self.contract_nodes(nodes)
            self.add_lines_to_component(node, lines)
            return lines

    def __str__(self):
        return str(self.component_list) + " and " + str(self.componentgraph.edges(data=True))
