from setuptools import setup, Extension

shortestline = Extension('shortestline', sources = ['shortestline.c'], depends = ['shortestline.c'])

setup(name = 'RoadNetwork',
      version = '0.1',
      description = 'Draw a road network',
      py_modules = ['roadmodel', 'roadscanvas', 'window'],
      scripts = ['test-closest-nonparallel.py', 'test-closest-parallel.py', 'test-roadmodel.py', 'test-short.py'],
      install_requires = ['networkx'],
      ext_package = 'shortestline',
      ext_modules = [shortestline])
