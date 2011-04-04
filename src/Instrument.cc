#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "Instrument.h"
#include "Date.h"

//////////////////////////////////////////
// Definition of InstrumentDefinition class
//////////////////////////////////////////
InstrumentDefinition::InstrumentDefinition(std::string maturity, int index):
    _maturity(maturity), _index(index)
{
}

InstrumentDefinition::~InstrumentDefinition()
{
}

bool InstrumentDefinition::cmp(const InstrumentDefinition& i1,
        const InstrumentDefinition& i2)
{
    int timeI1, timeI2;

    timeI1 = i1.maturityInDay();
    timeI2 = i2.maturityInDay();

    if(timeI1 == timeI2)
    {
        if((i1.type() == CASH &&
            (i2.type() == FRA || i2.type() == SWAP)) ||
                (i1.type() == FRA && i2.type() == SWAP))
            return true;
        else
            return false;
    }
    else
        return timeI1 < timeI2;

    return true;
}

bool InstrumentDefinition::ptrcmp(const InstrumentDefinition* pi1,
        const InstrumentDefinition* pi2)
{
    return InstrumentDefinition::cmp(*pi1, *pi2);
}

InstrumentDefinition* InstrumentDefinition::parseString(std::string& instrDefStr)
{
    try
    {
        std::string instrumentType;
        std::string dateStr;
        int id;

        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(",");
        tokenizer tokens(instrDefStr, sep);

        tokenizer::iterator tokIter = tokens.begin();
        instrumentType = *tokIter;

        tokIter ++;
        dateStr = *tokIter;
        
        tokIter ++;
        id = boost::lexical_cast<int>(*tokIter);

        if(boost::iequals(instrumentType, std::string("CASH")))
        {
            // InstrumentDefinitionType is CASH
            InstrumentDefinition *instr = new CASHInstrDefinition(dateStr, id);
            return instr;
        }
        else if(boost::iequals(instrumentType, std::string("FRA")))
        {
            // InstrumentDefinitionType is FRA
            boost::char_separator<char> FRADateSep("xX");
            tokenizer FRADateTokens(dateStr, FRADateSep);

            tokIter = FRADateTokens.begin();
            std::string startDuration = *tokIter;

            tokIter ++;
            std::string maturity = *tokIter;
            
            InstrumentDefinition *instr = new FRAInstrDefinition(startDuration, maturity, id);
            return instr;
        }
        else if(boost::iequals(instrumentType, std::string("SWAP")))
        {
            // InstrumentType is SWAP
            InstrumentDefinition *instr = new SWAPInstrDefinition(dateStr, id);
            return instr;
        }
        else
        {
            throw InstrumentException(instrDefStr);
        }
        
    }
    catch(std::istringstream::failure& e)
    {
        throw;
    }

}

//////////////////////////////////////////
// Definition of CASHInstrDefinition class
//////////////////////////////////////////
CASHInstrDefinition::CASHInstrDefinition(std::string& maturity, int index):
    InstrumentDefinition(maturity, index)
{
    _type = InstrumentDefinition::CASH;
}

CASHInstrDefinition::~CASHInstrDefinition()
{
}

int CASHInstrDefinition::maturityInDay() const
{
    return 0;
}

std::string CASHInstrDefinition::subtype() const
{
    return _maturity;
}
//////////////////////////////////////////
// Definition of FRAInstrDefinition class
//////////////////////////////////////////
FRAInstrDefinition::FRAInstrDefinition(std::string& startDuration,
        std::string& maturity, int index):
    _startDuration(startDuration), InstrumentDefinition(maturity, index)
{
    _type = InstrumentDefinition::FRA;
}

FRAInstrDefinition::~FRAInstrDefinition()
{
}

int FRAInstrDefinition::maturityInDay() const
{
    return 0;
}

std::string FRAInstrDefinition::subtype() const
{
    return _startDuration + "x" + _maturity;
}

//////////////////////////////////////////
// Definition of SWAPInstrDefinition class
//////////////////////////////////////////
SWAPInstrDefinition::SWAPInstrDefinition(std::string& maturity, int index):
    InstrumentDefinition(maturity, index)
{
    _type = InstrumentDefinition::SWAP;
}

SWAPInstrDefinition::~SWAPInstrDefinition()
{
}

int SWAPInstrDefinition::maturityInDay() const
{
    return 0;
}

std::string SWAPInstrDefinition::subtype() const
{
    return _maturity;
}

//////////////////////////////////////////
// Definition of InstrumentException class
//////////////////////////////////////////
InstrumentException::InstrumentException(std::string& str):
    _errorMessage(str)
{
}

InstrumentException::~InstrumentException()
{
}
