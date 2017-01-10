import argparse
import sys
import subprocess as sub
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
from xml.dom import minidom
import xml.etree.cElementTree as ET

## @namespace SideFunction  Namespace wrapping all python library
#
#==================================
#GlobalVariables
plotColors = [ 'black', 'red', 'blue', 'green', 'orange', 'brown' ]

#==================================
def GetLinearCoord( levelsSize, objCoords ) :
#coords from the most to least nested
    index = 0
    indexByStep = 1;
    for iObjCoords in range( 0, len( objCoords ) ) :
        index += indexByStep*objCoords[iObjCoords]
        indexByStep *= levelsSize[iObjCoords]

    return index

#==================================
def GetCoordFromLinear( levelSize, objIndex ) :
#coords from the most to least nested
    coords=[]
    subLevelSize=1
    for vLevel in levelSize :
        dumSubLevelSize = subLevelSize
        subLevelSize *= vLevel
        coords.append( (objIndex%subLevelSize)/dumSubLevelSize )

    return coords

#==================================
def PlotPoints( xPoints, yPoints, options={} ) :

    nPlots = len(xPoints)
    if len( yPoints ) != nPlots : print('yPoints size does not match x : xsize=',nPlots,' ysize=', len(yPoints) ); exit(0)

    if 'outName' not in options : options['outName'] = 'Plot.pdf'
    if not 'legend' in options : options['legend'] = []
    if len(options['legend']) and len(options['legend']) != nPlots : print('legend size does not match : xsize=', nPLots, ' legendsize=', len(options['legend']) ); options['legend'] = []    

    plt.ioff()
    fig = plt.figure()
    ax = fig.add_subplot(111)
    fig.subplots_adjust(bottom=0.1, top=0.95, right=0.95)
    fig.patch.set_facecolor('white')

    for iPlot in range(0, len(xPoints ) ) : 
        ax.plot( xPoints[iPlot], 
                 yPoints[iPlot],
                 color=plotColors[iPlot], 
                 linestyle= '-', 
                 label=options['legend'][iPlot] if len(options['legend']) else ''
                 )

    if 'yTitle' in options : ax.set_ylabel( options['yTitle' ] )
    if 'xTitle' in options : ax.set_xlabel( options['xTitle' ] )

    if 'annotate' in options : 
        for annotation in options['annotate' ] :
            plt.annotate(annotation['text'], xy=annotation['xy'], xytext=annotation['xytext'],
            arrowprops=dict(facecolor='black', shrink=0.05),
            )


    ax.legend(loc='upper right', frameon=False)
    y_formatter = matplotlib.ticker.ScalarFormatter(useOffset=False)
    ax.yaxis.set_major_formatter(y_formatter)

    #plt.show()

    fig.savefig( options['outName'] )
    return options['outName']

#==================================
def DSCB( myy, mu, sigma, alphaHi, alphaLow, nHi, nLow ) : 
    t = ( myy - mu ) / sigma;
    RLow = alphaLow/nLow
    RHi = alphaHi/nHi

    if t > alphaHi : 
        denom = RHi*(1/RHi-alphaHi+t)
        return np.exp( -alphaHi*alphaHi/2)/ np.power( denom, nHi )

    elif t < -alphaLow : 
        denom = (1/RLow-alphaLow-t)*RLow
        return np.exp( -alphaLow*alphaLow/2)/ np.power( denom, nLow )

    else : return np.exp( -t*t/2 )

#==================================
def listFiles( directory, pattern='' ):
    output = sub.check_output( ['ls '+ directory + pattern ],  shell=1, stderr=sub.STDOUT ) 
    if '\n' in output : content = output.split() 
    else : content = [ output ]
    return content

#==================================
def drawTabular( plots, title='', content='', width=0 ) :
    output = '\\begin{frame}{' + title + '}\n'

    plotsPerLine=2
    if len( plots ) == 2 or len( plots ) == 4 : plotsPerLine = 2
    elif len( plots ) == 1 : plotsPerLine = 1./0.7
    if not width : width = 0.9/plotsPerLine
    output += content + '\n'
    output += '\\centering\n'
    if len( plots ) ==1 : 
        output += ( '\\begin{adjustbox}{max width=' + str(width) +'\\textwidth}\\begin{tabular}{lllll}'
                    +'\\csvautotabular{' + plots[0] +'}\\end{tabular}'
                    + '\\end{adjustbox}\\\\' )

    else :
        output += '\n'.join( [ '\\begin{minipage}{' + str(width) + '\\linewidth}'
                               + '\\begin{adjustbox}{max width=\\textwidth}\\begin{tabular}{lllll}'
                               +'\\csvautotabular{' + plots[iPlot] +'}\\end{tabular}'
                               + '\\end{adjustbox}\\\\'
                               + '\\end{minipage}'
                               + ( '\\hfill' if iPlot != plotsPerLine-1 else '' )
                               for iPlot in range( 0, len(plots) ) 
                               ] )

    output += '\n\\end{frame}\n'
    return output

#==========================================
def parseArgs():
    parser = argparse.ArgumentParser(
        description="This text will be displayed when the -h or --help option are given"
                    "It should contain a short description of what this program is doing.")

    # parser.add_argument(
    #     '--doCorrection', help='Tag for recreating systematics histos and plots',
    #     default=0, type=int )
    parser.add_argument('inFiles', type=str, help="Directory where all inputs are stored", nargs='+' )

    args = parser.parse_args()
    return args

#=============================================
def StripString( line, doPrefix = 1, doSuffix = 1 ) :
    if ( line.rfind( '.' ) != -1 and doSuffix ) : line = line[0:line.rfind( '.' )]
    if ( line.rfind( '/' ) != -1 and doPrefix ) : line = line[line.rfind( '/' )+1:len( line )]
    return line

#=============================================
def LatexHeader( title, subtitle='', mode = 0 ) :
    output = ( '\\documentclass[a4paper]{beamer}\n' 
             +'\\usepackage[english]{babel}\n'
             +'\\usepackage[utf8]{inputenc}\n'
             +'\\usepackage{graphicx}\n' ) 
    if mode : 
        output += ( '\\usepackage{subcaption}\n'
                    +'\\usepackage{caption}\n'
                    +'\\usepackage{appendixnumberbeamer}\n'
                    +'\\usepackage{tikz}\n'
                    +'\\usepackage{csvsimple}\n'
                    +'\\usepackage{tabularx}\n'
                    +'\\usepackage{adjustbox}\n'
                    +'\\usepackage{pdfpages}\n'
                    +'\\usepackage{cleveref}\n'
                   # +'\\usepackage[orientation=paysage,size=A4]{beamerposter}\n'
                    +'\\captionsetup{labelformat=empty,labelsep=none}\n' )
    output += ( '\\usetheme{Boadilla}\n'
                +'\\beamertemplatenavigationsymbolsempty\n'
                +'\\title[' + subtitle +']{' + title + '}\n'
                +'\\author[Guerguichon]{Antinea Guerguichon}\n' )
    
    if mode : output += '\\institute[LAL]{\includegraphics[width=0.3\\linewidth]{LAL.jpg} }\n'
    else : output += '\\institute[LAL]{\includegraphics[width=0.3\\linewidth]{/afs/in2p3.fr/home/a/aguergui/LAL.jpg} }\n'
    #output += '\\institute[LAL]{\includegraphics[width=0.3\\linewidth]{/afs/in2p3.fr/home/a/aguergui/LAL.jpg} }\n' 
    output +=  ( '\\date{\\today}\n'
             +'\\begin{document}\n'
             +'\\transboxin\n'
             +'\\begin{frame}\n'
             +'\\maketitle\n'
             +'\\end{frame}\n'
             )
    return output

#============================================
def drawMinipage( plots, title='', content='' ) :
    if not len( plots ) : return ''
    output = '\\begin{frame}{' + title + '}\n'

    plotsPerLine=4
    width=0.24
    if len( plots ) == 1 : plotsPerLine = 1; width=0.7
    if len( plots ) == 2 : plotsPerLine = 2; width=0.49
    elif len( plots ) < 5 : plotsPerLine=2; width = 0.42
    elif len( plots ) < 10 : plotsPerLine=3; width = 0.32

    output += content + '\n'
    output += '\\centering\n'
    output += '\n'.join( [ '\\begin{minipage}{' + str(width) + '\\linewidth}'
                + '\\includegraphics[width=\\linewidth]{' + plots[iPlot] + '}'
                + '\\end{minipage}'
                + ( '\\hfill' if iPlot != plotsPerLine-1 else '' )
                for iPlot in range( 0, len(plots) ) 
                ] )

    output += '\n\\end{frame}\n'
    return output

#============================================
def AddSlash( string ) :
    return string + ( '/' if string[-1] != '/' else '' )

#===================================
def BatchHeader( path, package, macro ) :
    while path[-1] == '/' : path= path[:-1]
    return ( 'server=`pwd`\n' 
             + 'cd ${server} \n'
             + 'ulimit -S -s 100000 \n'
             + 'LD_LIBRARY_PATH=' + path + '/RootCoreBin/lib:' + path +'/RootCoreBin/bin:$LD_LIBRARY_PATH \n'
             + 'cd '+path+'/RootCoreBin/ \n'
             + 'source local_setup.sh \n'
             + 'cd ${server} \n'
             + 'cp -v '+path+'/RootCoreBin/obj/x86_64-slc6-gcc49-opt/'+package+'/bin/'+macro+' . \n'
             )

#======================================
def AbsPath( inFile ) :
    if inFile[0] == '/'  : return inFile
    output = sub.check_output( ['pwd' ],  shell=1, stderr=sub.STDOUT ).split('\n')[0]
    output = AddSlash( output )
    output+=inFile
    return output
    
#=================================================
def prettify(elem):
    """Return a pretty-printed XML string for the Element.
    """

    rough_string = ET.tostring(elem, 'utf-8')
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent="  ")
#================================================
def CreateNode( nodeName, options={} ) :
    xmlObj = ET.Element( nodeName ) 
    [ xmlObj.set( opt, options[opt] ) for opt in options ]
    return xmlObj
