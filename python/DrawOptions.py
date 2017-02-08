class DrawOptions :
    """
    A class to ease the writting of boost file for PlotDist macro.
    """
    
    def __init__(self) :
        keys = [ "xTitle", "yTitle", 'legendPos', 'rangeUserX', 'rangeUserY', 'varMin', 'varMax', 
                 "scale", "line", "clean", "normalize", "topMargin", "bottomMargin" "doChi2", "logy", "orderX", 
                 "doRatio", "drawStyle", "shiftColor", "grid", 'extendUp', 'offset', 'inputType', 'selectionCut',
                 'eventID', 'nEvents', 'nBins', 'triangular', 'xBinning', 'yBinning', 'xTitle', 'yTitle', 'doTabular',
                 'logy', 'removeVal', 'orderX', 'forceStyle', 'extension', 'outName', 'saveRoot', 'doLabels',
                 'plotDirectory']
        self.__singleOptions = { vKey:'' for vKey in keys }

        keys = ['rootFileName', 'objName', 'varName', 'varWeight', 'latex', 'latexOpt', 'legend', 'varYName', 
                'varErrX', 'varErrY', 'loadFiles', ]
        self.__multiOptions = { vKey:[] for vKey in keys }

    def WriteToFile( self, fileName ) :
        """
        Write the options to a text file.
        """
        outFile = open( fileName, 'w' )
        outFile.write( self.__SingleOptionsContent() +'\n')
        outFile.write( self.__MultiOptionsContent() )
        outFile.close()

    def Print(self) :
        print( self.__SingleOptionsContent() )
        print( self.__MultiOptionsContent() )


    def __SingleOptionsContent( self ) :
        return '\n'.join( [vKey+'='+vVal for vKey, vVal in self.__singleOptions.iteritems() if vVal!=''] )

    def __MultiOptionsContent( self ) :
        return '\n'.join( [vKey+'='+v2Val for vKey, vVal in self.__multiOptions.iteritems() for v2Val in vVal if v2Val!=''] )

    def AddOption( self, key, value ) :
        if self.__singleOptions.has_key(key) : self.__singleOptions[key] = str(value)
        elif self.__multiOptions.has_key(key) : self.__multiOptions[key].append( str(value) )
        else : print( key + ' is not an option.' )

    def ClearMulti( self, key ) :
        if self.__multiOptions.has_key(key) : self.__multiOptions[key] = []
        else : print( key + ' is not an option.' )
