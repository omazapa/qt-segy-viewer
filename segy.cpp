#include <QtGui>
#include <segy.h>
#include <util.h>

SEGY::SEGY()
{
    _F = NULL;

    _ByteLineName = 90;
    _LineNameLength = 10;
    _ByteCDP = 1;
    _ByteSP  = 17;
    _ByteX   = 73;
    _ByteY   = 77;
    _ByteInline = 9;
    _ByteXline  = 21;
    _MinInline = _MaxInline = _MinXline = _MaxXline = 0;
    _Corner1_X = _Corner1_Y = _Corner2_X = _Corner2_Y = _Corner3_X = _Corner3_Y = 0;
    _FirstCDP = _FirstSP = _FirstX = _FirstY = 0;
    _LastCDP = _LastSP = _LastX = _LastY = 0;
    _CDPInc = _SPInc = 0;
    _MinInline_Trace = _MaxInline_Trace = _MinXline_Trace = _MaxXline_Trace = 0;
}

bool SEGY::OpenFile(char* _Filename)
{
    long long l;

    if(_F)
        fclose(_F);

    _F = fopen64(_Filename, "rb");

    if(_F)
    {
        fseeko64(_F,0,SEEK_END);
        l = ftell(_F);
        l = ftello64(_F);

        fseek(_F,0,SEEK_SET);

        fread(_EbcdicHeader,1,3200,_F);
        fread(_BinaryHeader,1, 400,_F);

        _SampleInterval = i2(_BinaryHeader, 17)/1000000.;
        _TotalSamples = i2(_BinaryHeader, 21);
        _DataFormat = i2(_BinaryHeader, 25);
        _EndTime = _SampleInterval * _TotalSamples * 1000;

        if((_SampleInterval <= 0) || (_TotalSamples <= 0) || ((_DataFormat < 1) || (_DataFormat > 6)) ) goto ERR;

        if (_DataFormat == 3)
            _SampleRate = 2;
        else if(_DataFormat == 6)
            _SampleRate = 1;
        else
            _SampleRate = 4;

        _TRL = 240 + _TotalSamples * _SampleRate;

        if(_TRL <= 240)
            goto ERR;

        _TotalTraces = (l-3600)/(_TRL);

        return true;

        ERR:
            fclose(_F);
            _F = NULL;
            return false;
    }
    else
    {
        return false;
    }
}

void SEGY::ShowEbcdicHeader(QPlainTextEdit *ptxtEbcdic)
{
    if(_F==NULL) return;

    unsigned char str[128];
    int n;
    QString qstr;

    memset(str, 0, 128);

    ptxtEbcdic->clear();
    for(n=0; n<40; n++)
    {
        memcpy(str, (_EbcdicHeader + n * 80), 80);
        ebasd(str, str);
        qstr.sprintf("%s", str);
        ptxtEbcdic->appendPlainText(qstr);
    }
    ptxtEbcdic->verticalScrollBar()->setValue(0);
}

void SEGY::ShowBinaryHeader(QPlainTextEdit *ptxtBinary)
{
    if(_F==NULL) return;

    ptxtBinary->clear();

    QString qstr;
    int byte = 1;

    ptxtBinary->insertPlainText("Job identification number :......................" + QString::number(i4(_BinaryHeader, 1)) + "\n");
    ptxtBinary->insertPlainText("Line number :...................................." + QString::number(i4(_BinaryHeader, 5)) + "\n");
    ptxtBinary->insertPlainText("Reel number :...................................." + QString::number(i4(_BinaryHeader, 9)) + "\n");
    ptxtBinary->insertPlainText("# data traces per record :......................." + QString::number(i2(_BinaryHeader, 13)) + "\n");
    ptxtBinary->insertPlainText("# aux  traces per record :......................." + QString::number(i2(_BinaryHeader, 15)) + "\n");
    ptxtBinary->insertPlainText("Sample interval (microseconds) for reel :........" + QString::number(i2(_BinaryHeader, 17)) + "\n");
    ptxtBinary->insertPlainText("Sample interval (microseconds) for field :......." + QString::number(i2(_BinaryHeader, 19)) + "\n");
    ptxtBinary->insertPlainText("Number samples per data trace for reel :........." + QString::number(i2(_BinaryHeader, 21)) + "\n");
    ptxtBinary->insertPlainText("Number samples per datat trace for field :......." + QString::number(i2(_BinaryHeader, 23)) + "\n");
    ptxtBinary->insertPlainText("Data sample format code :........................" + QString::number(i2(_BinaryHeader, 25)) + "\n");
    ptxtBinary->insertPlainText("CDP fold :......................................." + QString::number(i2(_BinaryHeader, 27)) + "\n");
    ptxtBinary->insertPlainText("Trace sorting code :............................." + QString::number(i2(_BinaryHeader, 29)) + "\n");
    ptxtBinary->insertPlainText("Vertical sum code :.............................." + QString::number(i2(_BinaryHeader, 31)) + "\n");
    ptxtBinary->insertPlainText("Sweep frequency at start :......................." + QString::number(i2(_BinaryHeader, 33)) + "\n");
    ptxtBinary->insertPlainText("Sweep frequency at end :........................." + QString::number(i2(_BinaryHeader, 35)) + "\n");
    ptxtBinary->insertPlainText("Sweep length (milliseconds) :...................." + QString::number(i2(_BinaryHeader, 37)) + "\n");
    ptxtBinary->insertPlainText("Sweep type code :................................" + QString::number(i2(_BinaryHeader, 39)) + "\n");
    ptxtBinary->insertPlainText("Trace number of sweep channel :.................." + QString::number(i2(_BinaryHeader, 41)) + "\n");
    ptxtBinary->insertPlainText("Sweep trace taper length at start (ms) :........." + QString::number(i2(_BinaryHeader, 43)) + "\n");
    ptxtBinary->insertPlainText("Sweep trace taper length at end   (ms) :........." + QString::number(i2(_BinaryHeader, 45)) + "\n");
    ptxtBinary->insertPlainText("Taper type :....................................." + QString::number(i2(_BinaryHeader, 47)) + "\n");
    ptxtBinary->insertPlainText("Corellated data traces :........................." + QString::number(i2(_BinaryHeader, 49)) + "\n");
    ptxtBinary->insertPlainText("Binary gain recoverd :..........................." + QString::number(i2(_BinaryHeader, 51)) + "\n");
    ptxtBinary->insertPlainText("Amplitude recovery method :......................" + QString::number(i2(_BinaryHeader, 53)) + "\n");
    ptxtBinary->insertPlainText("Measuriment system (1-m / 2-feet) :.............." + QString::number(i2(_BinaryHeader, 55)) + "\n");
    ptxtBinary->insertPlainText("Impulse signal :................................." + QString::number(i2(_BinaryHeader, 57)) + "\n");
    ptxtBinary->insertPlainText("Vibratory polarity code :........................" + QString::number(i2(_BinaryHeader, 59)) + "\n\n");

    for(int i=1; i<=3; i++)
    {
        qstr.sprintf("%3d  %-15d\t",byte,i4(_BinaryHeader,byte));
        ptxtBinary->insertPlainText(qstr);
        byte += 4;
    }

    for(int i=1; i<=15; i++)
    {
        ptxtBinary->insertPlainText("\n");
        for(int j=1; j<=3; j++)
        {
            qstr.sprintf("%3d  %-15d\t",byte,i2(_BinaryHeader,byte));
            ptxtBinary->insertPlainText(qstr);
            byte += 2;
        }
    }
}

float SEGY::GetSample(int idx)
{
    float   smp;
    int*    xmp = (int*)&smp;
    float*  dat = (float*)(_INPTRC+240);

    short*       int2ptr = (short*)       dat;
    int*         int4ptr = (int*)         dat;
    signed char* int1ptr = (signed char*) dat;

    if(_DataFormat==1)
     {
        smp =  dat[idx];
        ibm2ieee(&smp,1);
     }
    else if(_DataFormat==2)
     {
        smp = swapi4(int4ptr[idx]);
     }
    else if(_DataFormat==3)
     {
        smp = swapi2(int2ptr[idx]);
     }
    else if(_DataFormat==4)
     {
        smp = dat[idx];
     }
    else if(_DataFormat==5)
     {
        *xmp = swapi4(int4ptr[idx]);
     }
    else if(_DataFormat==6)
    {
        smp = int1ptr[idx];
    }
    else
        smp = 0;

    return smp;
}

bool SEGY::ReadTrace(long long n)
{
    long long l;

    if(_F == NULL) return false;

    if(n == 0) return false;

    if(n < 1 || n > _TotalTraces)
    {
        printf("Bad Trace Number");
        return false;
    }

    fseeko64(_F, 3600+(n-1)*_TRL, SEEK_SET);
    l = fread(_INPTRC, 1, _TRL, _F );

    _CurrentTrace = n;

    for(n=0; n < _TotalSamples; n++)
        _Data[n] =  GetSample(n);

    return true;
}

void SEGY::ShowTraceHeader(long long _TraceNo, QPlainTextEdit *ptxtTrace)
{
    ptxtTrace->clear();

    if(!ReadTrace(_TraceNo)) return;

    if(_F==NULL) return;

    ptxtTrace->insertPlainText("Trace # " + QString::number(_CurrentTrace) + "\n\n");

    QString qstr;

    int atByte[] = {     1,   5,   9,  13,  17,  21,  25,  29,  31, 33,
                        35,  37,  41,  45,  49,  53,  57,  61,  65,
                        69,  73,  77,  81,  85,  89,  91,  93,  95,
                        97,  99, 101, 103, 105, 107, 109, 111, 113,
                       115, 117, 119, 121, 123, 125, 127, 129, 131,
                       133, 135, 137, 139, 141, 143, 145, 147, 149,
                       151, 153, 155, 157, 159, 161, 163, 165, 167,
                       169, 171, 173, 175, 177, 179, 181, 183, 185,
                       187, 189, 191, 193, 195, 197, 199, 201, 203,
                       205, 207, 209, 211, 213, 215, 217, 219, 221,
                       223, 225, 227, 229, 231, 233, 235, 237, 239,  0
                   };
    int readType[] =  {   4,  4,  4,  4,  4,  4,  4,  2,  2,  2,
                        2,  4,  4,  4,  4,  4,  4,  4,  4,
                        4,  4,  4,  4,  4,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2
                    };


    for(int i=0; atByte[i]; i++)
    {
        qstr.sprintf("%3d  %-10d\t", atByte[i], (readType[i] == 2)? i2(_INPTRC, atByte[i]) : i4(_INPTRC, atByte[i]));
        ptxtTrace->insertPlainText(qstr);
        if(!((i+1)%4)) ptxtTrace->insertPlainText("\n");
    }
}

bool SEGY::FileIsOpen()
{
    if(_F != NULL) return true;
    else return false;
}

void SEGY::ReadFirstAndLast()
{
    int readType[] =  {   4,  4,  4,  4,  4,  4,  4,  2,  2,  2,
                        2,  4,  4,  4,  4,  4,  4,  4,  4,
                        4,  4,  4,  4,  4,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2,
                        2,  2,  2,  2,  2,  2,  2,  2,  2
                    };

    ReadTrace(1);
    _FirstCDP = (readType[_ByteCDP] == 2)? i2(_INPTRC, _ByteCDP) : i4(_INPTRC, _ByteCDP);
    _FirstSP = (readType[_ByteSP] == 2)? i2(_INPTRC, _ByteSP) : i4(_INPTRC, _ByteSP);
    _FirstX = i4(_INPTRC, _ByteX);
    _FirstY = i4(_INPTRC, _ByteY);

    ReadTrace(_TotalTraces);
    _LastCDP = (readType[_ByteCDP] == 2)? i2(_INPTRC, _ByteCDP) : i4(_INPTRC, _ByteCDP);
    _LastSP = (readType[_ByteSP] == 2)? i2(_INPTRC, _ByteSP) : i4(_INPTRC, _ByteSP);
    _LastX = i4(_INPTRC, _ByteX);
    _LastY = i4(_INPTRC, _ByteY);

    _CDPInc = (double)(_LastCDP - _FirstCDP + 1) / (double)_TotalTraces;
    _SPInc = (double)(_LastSP - _FirstSP + 1) / (double)_TotalTraces;

    //Line Name
    int n = _ByteLineName / 80;
    int m = _ByteLineName % 80;
    unsigned char str[128];
    memset(str, 0, 128);
    memcpy(str, (_EbcdicHeader + n * 80 + m), _LineNameLength);
    ebasd(str, str);
    _LineName.sprintf("%s", str);
}

void SEGY::SetByte(int cdp, int sp, int x, int y, int il, int xl, int name, int length)
{
    _ByteCDP = cdp;
    _ByteSP  = sp;
    _ByteX   = x;
    _ByteY   = y;
    _ByteInline = il;
    _ByteXline  = xl;
    _ByteLineName = name;
    _LineNameLength = length;
}

int SEGY::Read2Byte(long long trace, int byte)
{
    if(!ReadTrace(trace)) return 0;
    return i2(_INPTRC, byte);
}

int SEGY::Read4Byte(long long trace, int byte)
{
    if(!ReadTrace(trace)) return 0;
    return i4(_INPTRC, byte);
}

void SEGY::computeILXLRange()
{
    //Read at _ByteInline and from 1 to _TotalTraces
    int tmp = 0;
    for(long long trace=1; trace<=_TotalTraces; trace++)
    {
        tmp = Read4Byte(trace, _ByteInline);
        if(_MinInline == 0 && _MaxInline == 0) { _MinInline = tmp; _MinInline_Trace = trace; _MaxInline = tmp; _MaxInline_Trace = trace;}
        if(tmp < _MinInline)
        {
            // First Inline changed
            _MinInline = tmp;
            _MinInline_Trace = trace;
        }
        else if(tmp > _MaxInline)
        {
            // Last Inline changed
            _MaxInline = tmp;
            _MaxInline_Trace = trace;
        }
        tmp = Read4Byte(trace, _ByteXline);
        if(_MinXline == 0 && _MaxXline == 0) { _MinXline = tmp; _MinXline_Trace = trace; _MaxXline = tmp; _MaxXline_Trace = trace; }
        if(tmp < _MinXline)
        {
            // First Xline changed
            _MinXline = tmp;
            _MinXline_Trace = trace;
        }
        else if(tmp > _MaxXline)
        {
            // Last Xline changed
            _MaxXline = tmp;
            _MaxXline_Trace = trace;
        }
    }
}

void SEGY::computeCorners()
{
    int p1_inline, p1_xline, p1_x, p1_y;
    int p2_inline, p2_xline, p2_x, p2_y;
    int p3_inline, p3_xline, p3_x, p3_y;

    p1_inline = Read4Byte(_MinInline_Trace, _ByteInline);
    p1_xline = Read4Byte(_MinInline_Trace, _ByteXLine);
    p1_x = Read4Byte(_MinInline_Trace, _ByteX);
    p1_y = Read4Byte(_MinInline_Trace, _ByteY);
    p2_inline = p1_inline;
    int i = _MinInline_Trace + 1;
    while(p2_inline == Read4Byte(i, _ByteInline))
    {
        p2_xline = Read4Byte(i, _ByteXline);
        p2_x = Read4Byte(i, _ByteX);
        p2_y = Read4Byte(i, _ByteY);
        i++;
    }
    p3_inline = Read4Byte(_MaxInline_Trace, _ByteInline);
    p3_xline = Read4Byte(_MaxInline_Trace, _ByteXline);
    p3_x = Read4Byte(_MaxInline_Trace, _ByteX);
    p3_y = Read4Byte(_MaxInline_Trace, _ByteY);

    //find delta x and delta y for inline and xline
    int inline_delta   = p1_inline - p2_inline;
    int inline_delta_x = p1_x - p2_x;
    int inline_delta_y = p1_y - p2_y;
    int xline_delta    = p1_inline - p3_inline;
    int xline_delta_x  = p1_x - p3_x;
    int xline_delta_y  = p1_y - p3_y;

    int diff;
    // Corner 1
    if(p1_inline != _MinInline)
    {
        diff = p1_inline - _MinInline;
        p1_x = p1_x + ((diff * inline_delta_x)/inline_delta);
        p1_y = p1_y + ((diff * inline_delta_y)/inline_delta);
    }
    if(p1_xline != _MinXline)
    {
        diff = p1_xline - _MinXline;
        p1_x = p1_x + ((diff * xline_delta_x)/xline_delta);
        p1_y = p1_y + ((diff * xline_delta_y)/xline_delta);
    }
    _Corner1_X = p1_x;
    _Corner1_Y = p1_y;
    // Corner 2
    if(p2_inline != _MaxInline)
    {
        diff = _MaxInline - p2_inline;
        p2_x = p2_x + ((diff * inline_delta_x)/inline_delta);
        p2_x = p2_y + ((diff * inline_delta_y)/inline_delta);
    }
    if(p2_xline != _MaxXline)
    {
        diff = _MaxXline - p2_xline;
        p2_x = p2_x + ((diff * xline_delta_x)/xline_delta);
        p2_y = p2_y + ((diff * xline_delta_y)/xline_delta);
    }
    _Corner2_X = p2_x;
    _Corner2_Y = p2_y;
    // Corner 3
    if(p3_inline != _MinInline)
    {
        diff = p3_inline - _MinInline;
        p3_x = p3_x + ((diff * inline_delta_x)/inline_delta);
        p3_y = p3_y + ((diff * inline_delta_y)/inline_delta);
    }
    if(p3_xline != _MinXline)
    {
        diff = p3_xline - _MinXline;
        _Corner3_X = p3_x + ((diff * xline_delta_x)/xline_delta);
        _Corner3_Y = p3_y + ((diff * xline_delta_y)/xline_delta);
    }
    _Corner3_X = p3_x;
    _Corner3_Y = p3_y;
    // Corner 4
    int p4_x, p4_y;
    diff = _MaxXline - P ;
    _Corner4_X = p3_x + ((diff * xline_delta_x)/xline_delta);
    _Corner4_Y = p3_y + ((diff * xline_delta_y)/xline_delta);
    diff = _MaxXline - p3_xline;
    _Corner4_X = p3_x + ((diff * xline_delta_x)/xline_delta);
    _Corner4_Y = p3_y + ((diff * xline_delta_y)/xline_delta);
}
