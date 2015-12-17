#!/usr/bin/python

from osgeo import gdal
import numpy as np

# filenames to process
filename = "imgn41w122_13.img"
outputname = "imgn41w122_13.raw"

# read the raster as a numpy array
raster = gdal.Open(filename)
dem = raster.ReadAsArray()

f = open(outputname, 'wb')
f.write(dem.reshape(dem.shape[0]*dem.shape[1]))
f.close()

