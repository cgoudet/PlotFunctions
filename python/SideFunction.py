import argparse
import sys
import subprocess as sub


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
                    +'\\usepackage[orientation=paysage,size=A4]{beamerposter}\n'
                    +'\\captionsetup{labelformat=empty,labelsep=none}\n' )
    output += ( '\\usetheme{Boadilla}\n'
                +'\\beamertemplatenavigationsymbolsempty\n'
                +'\\title[' + subtitle +']{' + title + '}\n'
                +'\\author[Goudet]{Christophe Goudet}\n' )
    
    if mode : output += '\\institute[LAL]{\includegraphics[width=0.3\\linewidth]{/home/goudet/Hgg/ExternalPlot/LAL.jpg} }\n'
    else : output += '\\institute[LAL]{\includegraphics[width=0.3\\linewidth]{/afs/in2p3.fr/home/c/cgoudet/LAL.jpg} }\n'
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
# def listFiles( directory ):
#     output = sub.check_output( ['ls '+ directory ],  shell=1, stderr=sub.STDOUT ) 
#     if '\n' in output : content = output.split() 
#     else : content = [ output ]
#     return content

#=================================
def addSlash( string ) :
    return string + ( '/' if string[-1] != '/' else '' )

#===================================
