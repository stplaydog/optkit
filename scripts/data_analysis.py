#
# Copy right YMSys, 2015,2016 Zhaoming Yin
#
# @brief    This script performs data analytics
#           1) it can list:
#              name, numV, numE, numCC, avgDiam, varDiam, avgCluCoeff, varCluCoeff
#           2) it can draw distribution of 
#              clique, truss
#           
#
#   MODIFIED   (MM/DD/YY)
#   stplaydog   10/21/16 - Clustering coefficient analysis
#   stplaydog   08/27/16 - Add data plot functions 
#   stplaydog   08/20/16 - Implementation 
#   stplaydog   08/07/16 - Creation
#


import sys
import json
import numpy
import datetime
import time
import argparse
from enum import Enum
import glob, os
import re
from os.path import basename

import math
import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot as plt
from scipy.interpolate import spline
from pandas import DataFrame

import pandas as pd
import rpy2.robjects as robj
import rpy2.robjects.pandas2ri # for dataframe conversion
from rpy2.robjects.packages import importr
from rpy2.robjects.lib import ggplot2

from subprocess import call

import os


class JsonStats:

    def __init__(self, file):

        with open(file) as data_file:    
            data = json.load(data_file)
        
        #name_items       = basename(file).replace(".json", "").split("_")
        #self.name        = "long_" if name_items[2] == "200" else "short_" 
        self.name        = "" 
        self.numV        = data["content"]["graph property"]["numV"] 
        self.numE        = data["content"]["graph property"]["numE"]
        self.numCC       = data["content"]["graph property"]["numCC"] 
        numDiam          = data["content"]["graph property"]["diameter"].split(",") 
        LDiam            = [float(n) for n in numDiam if n] 
        self.avgDiam     = str(numpy.average(LDiam)) 
        self.varDiam     = str(numpy.var(LDiam))
        numClu           = data["content"]["graph property"]["clusterCoeff"].split(",") 
        LClu             = [float(n) for n in numClu if n] 
        self.avgCluCoeff = str(numpy.average(LClu))
        self.varCluCoeff = str(numpy.var(LClu))

        self.clique      = self.reduce(data["content"]["graph property"]["clique"], True)
        self.truss       = self.reduce(data["content"]["graph property"]["truss"],  True)
        self.core        = self.reduce(data["content"]["graph property"]["dbscan"], True)
        self.dbscan      = self.reduce(data["content"]["graph property"]["core"],   True)

        self.cliqueSize  = self.reduce(data["content"]["graph property"]["clique"], False)
        self.trussSize   = self.reduce(data["content"]["graph property"]["truss"],  False)
        self.coreSize    = self.reduce(data["content"]["graph property"]["dbscan"], False)
        self.dbscanSize  = self.reduce(data["content"]["graph property"]["core"],   False)

        self.cliqueSize  = self.getSizeMean(self.clique, self.cliqueSize)
        self.trussSize   = self.getSizeMean(self.truss,  self.trussSize)
        self.coreSize    = self.getSizeMean(self.core,   self.coreSize)
        self.dbscanSize  = self.getSizeMean(self.dbscan, self.dbscanSize)
        
        self.trussCoe    = self.reduce(data["content"]["graph property"]["truss_coe"],  False)
        self.coreCoe     = self.reduce(data["content"]["graph property"]["dbscan_coe"], False)
        self.dbscanCoe   = self.reduce(data["content"]["graph property"]["core_coe"],   False)

        self.trussCoe    = self.getSizeMean(self.truss,  self.trussCoe)
        self.coreCoe     = self.getSizeMean(self.core,   self.coreCoe)
        self.dbscanCoe   = self.getSizeMean(self.dbscan, self.dbscanCoe)


    def reduce(self, stats_str, if_freq):
        stats_item = {} 
        items = stats_str.split("\n")
        for item in items:
            if item == "":
                continue
            pair = item.split(",")
            if int(pair[0]) in stats_item:
                if if_freq:
                    stats_item[int(pair[0])] += 1 
                else:
                    stats_item[int(pair[0])] += float(pair[1])
            else:
                if if_freq:
                    stats_item[int(pair[0])] = 1 
                else:
                    stats_item[int(pair[0])] = float(pair[1])
        X = [0] * len(stats_item)
        Y = [0] * len(stats_item)
        i=0
        for key in stats_item:
            X[i] = int(key)
            Y[i] = stats_item[key]
            i+=1

        return {'x':X,'y':Y} 

    def getSizeMean(self, freq, size):
        for i in range(0, len(freq['y'])):
            size['y'][i] = float(size['y'][i]) / float(freq['y'][i])
        return size

    def smooth_plot(self, item, plt, c, ls, mar, la):
        if len(item['x']) == 0:
            return
        arr = numpy.array(item['x'])
        xnew   = numpy.linspace(arr.min(),arr.max(),300)
        smooth = spline(item['x'], item['y'], xnew) 
        plt.plot(xnew, smooth, color=c, linestyle=ls, marker=mar, label = la)



    def plot(self, ofname):
        plt.plot(self.clique['x'], self.clique['y'], color='k', linestyle='-', marker=',', label = 'k-clique')
        plt.plot(self.truss['x'],  self.truss['y'],  color='k', linestyle='-', marker='.', label = 'k-truss')
        plt.plot(self.dbscan['x'], self.clique['y'], color='k', linestyle='-', marker='v', label = 'dbscan')
        plt.plot(self.core['x'],   self.core['y'],   color='k', linestyle='-', marker='o', label = 'k-core')
        plt.legend( loc='lower right', numpoints = 1, prop={'size':15} )
        plt.tick_params(labelsize=15)
        plt.xlabel("K", fontsize=20)
        plt.ylabel("number of cohesive subgraphs", fontsize=20)
        plt.tight_layout()
        plt.savefig(ofname)
        plt.close()

    def summary(self):
        list = [self.name,             str(self.numV),    str(self.numE), \
                str(self.numCC),       str(round(self.avgDiam,2)), str(round(self.varDiam,2)), \
                str(round(self.avgCluCoeff,2)), str(round(self.varCluCoeff,2)) ]
        return ",".join(list)

class JsonStatsCollections:

    def __init__(self, dir, prefix):
        os.chdir(dir)
        self.coll = {}
        for file in glob.glob("*.json"):
            try:
                if file.find(prefix) != -1:
                    stats = JsonStats(file)  
                    self.coll[file] = stats
            except Exception, e:
                print e
                print "Data Corruption in " + file

    def plot(self, ofname, is_freq):
        colors = ['k', 'b', 'r', 'g']
        i = 0
        for c in self.coll: 
            if is_freq == False:
                self.coll[c].smooth_plot(self.coll[c].cliqueSize, plt, colors[i], '--', ',', self.coll[c].name+'-clique')
                self.coll[c].smooth_plot(self.coll[c].trussSize,  plt, colors[i], '--', '.', self.coll[c].name+'-truss')
                self.coll[c].smooth_plot(self.coll[c].coreSize,   plt, colors[i], '-',  'v', self.coll[c].name+'-core')
                self.coll[c].smooth_plot(self.coll[c].dbscanSize, plt, colors[i], '-',  'o', self.coll[c].name+'-dbscan')
            elif is_freq == True:
                plt.plot(self.coll[c].clique['x'], self.coll[c].clique['y'], color=colors[i], linestyle='--', marker=',', label = self.coll[c].name+'-clique')
                plt.plot(self.coll[c].truss['x'],  self.coll[c].truss['y'],  color=colors[i], linestyle='--', marker='.', label = self.coll[c].name+'-truss')
                plt.plot(self.coll[c].core['x'],   self.coll[c].core['y'],   color=colors[i], linestyle='-',  marker='v', label = self.coll[c].name+'-core')
                plt.plot(self.coll[c].dbscan['x'], self.coll[c].dbscan['y'], color=colors[i], linestyle='-',  marker='o', label = self.coll[c].name+'-dbscan')
            i += 1
        plt.legend( loc=0, numpoints = 1, prop={'size':15} )
        plt.tick_params(labelsize=15)
        plt.xlabel("K", fontsize=20)
        plt.ylabel("number of cohesive subgraphs", fontsize=20)
        plt.tight_layout()
        plt.savefig(ofname)
        plt.close()

    def gplot(self, ofname, is_freq):

        i = 0
        d = []
        for c in self.coll: 
            if is_freq == 1:
                d = self.transformDataGgPlot(c, d)
            elif is_freq == 2:
                d = self.transformDataGgPlotSize(c, d)
            elif is_freq == 3:
                d = self.transformDataGgPlotCoe(c, d)
        f = DataFrame(d)
        print ofname
        f.to_csv(ofname.replace("png", "csv"), sep=',')

        call(["Rscript", "../../../scripts/data_analysis.R", ofname.replace("png", "csv"), ofname ])



    def transformDataGgPlotSize(self, c, ret):
        item = self.coll[c].trussSize
        for i in range(0, len(item['x'])):
            trip = {'data': self.coll[c].name+'truss', 'x': item['x'][i], 'y' : item['y'][i]}
            ret.append(trip)

        item = self.coll[c].cliqueSize
        for i in range(0, len(item['x'])):
            trip = {'data': self.coll[c].name+'clique', 'x': item['x'][i], 'y' : item['y'][i]}
            ret.append(trip)

        item = self.coll[c].coreSize
        for i in range(0, len(item['x'])):
            trip = {'data': self.coll[c].name+'core', 'x': item['x'][i], 'y' : item['y'][i]}
            ret.append(trip)

        item = self.coll[c].dbscanSize
        for i in range(0, len(item['x'])):
            trip = {'data': self.coll[c].name+'dbscan', 'x': item['x'][i], 'y' : item['y'][i]}
            ret.append(trip)
            
    def transformDataGgPlotCoe(self, c, ret):
        item = self.coll[c].trussCoe
        for i in range(0, len(item['x'])):
            trip = {'data': self.coll[c].name+'truss_coe', 'x': item['x'][i], 'y' : item['y'][i]}
            ret.append(trip)

        item = self.coll[c].coreCoe
        for i in range(0, len(item['x'])):
            trip = {'data': self.coll[c].name+'core_coe', 'x': item['x'][i], 'y' : item['y'][i]}
            ret.append(trip)

        item = self.coll[c].dbscanCoe
        for i in range(0, len(item['x'])):
            trip = {'data': self.coll[c].name+'dbscan_coe', 'x': item['x'][i], 'y' : item['y'][i]}
            ret.append(trip)
        return ret

    def transformDataGgPlot(self, c, ret):
        item = self.coll[c].truss
        for i in range(0, len(item['x'])):
            trip = {'data': self.coll[c].name+'truss', 'x': item['x'][i], 'y' : item['y'][i]}
            ret.append(trip)

        item = self.coll[c].clique
        for i in range(0, len(item['x'])):
            trip = {'data': self.coll[c].name+'clique', 'x': item['x'][i], 'y' : item['y'][i]}
            ret.append(trip)

        item = self.coll[c].core
        for i in range(0, len(item['x'])):
            trip = {'data': self.coll[c].name+'core', 'x': item['x'][i], 'y' : item['y'][i]}
            ret.append(trip)

        item = self.coll[c].dbscan
        for i in range(0, len(item['x'])):
            trip = {'data': self.coll[c].name+'dbscan', 'x': item['x'][i], 'y' : item['y'][i]}
            ret.append(trip)
        return ret
        

def main(argv):

    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group()
    group.add_argument("-f", "--file", action="store_true")
    group.add_argument("-d", "--directory", action="store_true")
    group.add_argument("-p", "--prefix", action="store_true")
    parser.add_argument("fname", help="file/directory name")
    args = parser.parse_args()

    if args.file:
        stats  = JsonStats(args.fname)
        print stats.summary()
        ofname = args.fname.replace('json', '') + 'png'
        stats.plot(ofname)
    elif args.directory:
        os.chdir(args.fname)
        for file in glob.glob("*.json"):
            try:
                stats = JsonStats(file)  
                print stats.summary()
                ofname = file.replace("json", "") + "png"
                stats.plot(ofname)
            except:
                print "Data Corruption in " + file
    elif args.prefix:
        config = open(args.fname)
        lines = config.readlines()
        for line in lines:
            if line.find("directory") != -1:
                dir = line.strip().split(" ")[1]
            if line.find("prefix") != -1:
                pfx = line.strip().split(" ")[1]
        coll = JsonStatsCollections(dir, pfx)
        oname1 = dir + pfx + '.png'
        oname2 = dir + pfx + '_size.png'
        oname3 = dir + pfx + '_coe.png'
        #coll.plot(oname2, False)
        #coll.plot(oname1, True)
        coll.gplot(oname1, 1)
        coll.gplot(oname2, 2)
        coll.gplot(oname3, 3)

if __name__ == "__main__":
    main(sys.argv)
