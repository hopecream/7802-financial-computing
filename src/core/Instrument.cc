#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include "Instrument.h"
#include "Date.h"

//////////////////////////////////////////
// Definition of InstrumentDefinition class
//////////////////////////////////////////
InstrumentDefinition::InstrumentDefinition(
        const Duration& maturity, const int index):
    _maturity(maturity), _index(index)
{
}

InstrumentDefinition::InstrumentDefinition(
        const InstrumentDefinition& rhs):
    _index(rhs._index), _maturity(rhs._maturity),
    _type(rhs._type)
{
}

InstrumentDefinition::~InstrumentDefinition()
{
}

std::string InstrumentDefinition::typeToString(
        InstrumentDefinition::TYPE type)
{
    switch(type)
    {
        case InstrumentDefinition::CASH:
            return std::string("CASH");
        case InstrumentDefinition::FRA:
            return std::string("FRA");
        case InstrumentDefinition::SWAP:
            return std::string("SWAP");
        case InstrumentDefinition::FAKE:
            return std::string("FAKE");
        default:
            {
                std::string errorMessage("Invalid Instrument Definition Type");
                throw InstrumentException(errorMessage);
            }
    }
}

InstrumentDefinition* InstrumentDefinition::parseString(std::string& instrDefStr)
{
    try
    {
        std::string trimedString = instrDefStr;
        std::string instrumentType;
        std::string dateStr;
        int id;

        boost::trim_if(trimedString, boost::algorithm::is_any_of("\r\n"));

        typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
        boost::char_separator<char> sep(",");
        tokenizer tokens(trimedString, sep);

        tokenizer::iterator tokIter = tokens.begin();
        if(tokIter == tokens.end())
            throw InstrumentException(instrDefStr);
        instrumentType = *tokIter;

        tokIter ++;
        if(tokIter == tokens.end())
            throw InstrumentException(instrDefStr);
        dateStr = *tokIter;
        
        tokIter ++;
        if(tokIter == tokens.end())
            throw InstrumentException(instrDefStr);
        id = boost::lexical_cast<int>(*tokIter);

        tokIter ++;
        if(tokIter != tokens.end())
            throw InstrumentException(instrDefStr);

        if(boost::iequals(instrumentType, std::string("CASH")))
        {
            // InstrumentDefinitionType is CASH
            Duration duration(dateStr);
            InstrumentDefinition *instr = new CASHInstrDefinition(duration, id);
            return instr;
        }
        else if(boost::iequals(instrumentType, std::string("FRA")))
        {
            // InstrumentDefinitionType is FRA
            boost::char_separator<char> FRADateSep("xX");
            tokenizer FRADateTokens(dateStr, FRADateSep);

            tokIter = FRADateTokens.begin();
            if(tokIter == FRADateTokens.end())
                throw InstrumentException(instrDefStr);
            std::string startDurationStr = *tokIter;

            tokIter ++;
            if(tokIter == FRADateTokens.end())
                throw InstrumentException(instrDefStr);
            std::string maturityStr = *tokIter;
            
            tokIter ++;
            if(tokIter != FRADateTokens.end())
                throw InstrumentException(instrDefStr);

            // Check if the Unit is provided or not
            static const boost::regex allDigitsFormat("^[[:digit:]]+$");
            Duration tStartDuration, tMaturity;
            
            if(boost::regex_match(startDurationStr, allDigitsFormat))
            {
                double startDurationNum = boost::lexical_cast<double>(startDurationStr);
                Duration startDuration(startDurationNum, FRAInstrDefinition::defaultDurationType);
                tStartDuration = startDuration;
            }
            else
            {
                Duration startDuration(startDurationStr);
                tStartDuration = startDuration;
            }

            if(boost::regex_match(maturityStr, allDigitsFormat))
            {
                double maturityNum = boost::lexical_cast<double>(maturityStr);
                Duration maturity(maturityNum, FRAInstrDefinition::defaultDurationType);
                tMaturity = maturity;
            }
            else
            {
                Duration maturity(maturityStr);
                tMaturity = maturity;
            }

            InstrumentDefinition *instr = new FRAInstrDefinition(tStartDuration, tMaturity, id);
            return instr;
        }
        else if(boost::iequals(instrumentType, std::string("SWAP")))
        {
            // InstrumentType is SWAP
            Duration duration(dateStr);

            InstrumentDefinition *instr = new SWAPInstrDefinition(duration, id);
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
    catch(boost::bad_lexical_cast& e)
    {
        throw InstrumentException(instrDefStr);
    }

}

InstrumentDefinition* InstrumentDefinition::clone()
{
    InstrumentDefinition *newInstrDef;
    switch(this->type())
    {
        case InstrumentDefinition::CASH:
            {
                CASHInstrDefinition* instrDef = 
                    dynamic_cast<CASHInstrDefinition *>(this);

                newInstrDef = new CASHInstrDefinition(*instrDef);
                break;
            }
        case InstrumentDefinition::FRA:
            {
                FRAInstrDefinition* instrDef =
                    dynamic_cast<FRAInstrDefinition *>(this);

                newInstrDef = new FRAInstrDefinition(*instrDef);
                break;
            }
        case InstrumentDefinition::SWAP:
            {
                SWAPInstrDefinition* instrDef =
                    dynamic_cast<SWAPInstrDefinition *>(this);

                newInstrDef = new SWAPInstrDefinition(*instrDef);
                break;
            }
        case InstrumentDefinition::FAKE:
            {
                FAKEInstrDefinition* instrDef = 
                    dynamic_cast<FAKEInstrDefinition *>(this);

                newInstrDef = new FAKEInstrDefinition(*instrDef);
                break;
            }
    }

    return newInstrDef;
}

//////////////////////////////////////////
// Definition of CASHInstrDefinition class
//////////////////////////////////////////
CASHInstrDefinition::CASHInstrDefinition(Duration& maturity, int index):
    InstrumentDefinition(maturity, index)
{
    _type = InstrumentDefinition::CASH;
}

CASHInstrDefinition::~CASHInstrDefinition()
{
}

std::string CASHInstrDefinition::subtype() const
{
    return _maturity.toString(true, true);
}
//////////////////////////////////////////
// Definition of FRAInstrDefinition class
//////////////////////////////////////////
const Duration::TYPE FRAInstrDefinition::defaultDurationType = Duration::MONTH;

FRAInstrDefinition::FRAInstrDefinition(Duration& startDuration,
        Duration& maturity, int index):
    _startDuration(startDuration), InstrumentDefinition(maturity, index)
{
    _type = InstrumentDefinition::FRA;
}

FRAInstrDefinition::~FRAInstrDefinition()
{
}

std::string FRAInstrDefinition::subtype() const
{
    return _startDuration.toString(false, false) + 
        "x" + _maturity.toString(false, false);
}

//////////////////////////////////////////
// Definition of SWAPInstrDefinition class
//////////////////////////////////////////
SWAPInstrDefinition::SWAPInstrDefinition(Duration& maturity, int index):
    InstrumentDefinition(maturity, index)
{
    _type = InstrumentDefinition::SWAP;
}

SWAPInstrDefinition::~SWAPInstrDefinition()
{
}

std::string SWAPInstrDefinition::subtype() const
{
    return _maturity.toString(true, true);
}

//////////////////////////////////////////
// Definition of SWAPInstrDefinition class
//////////////////////////////////////////
std::string FAKEInstrDefinition::subtype() const
{
    return _maturity.toString(true, true);
}

//////////////////////////////////////////
// Definition of InstrumentDefinitionCompare structure
//////////////////////////////////////////
bool InstrumentDefinitionCompare::operator()(
        const InstrumentDefinition& lhs,
        const InstrumentDefinition& rhs) const
{
    int timeI1, timeI2;

    timeI1 = lhs.maturity().getDuration(Duration::DAY);
    timeI2 = rhs.maturity().getDuration(Duration::DAY);

    if(timeI1 == timeI2)
    {
        if((lhs.type() == InstrumentDefinition::CASH &&
            (rhs.type() == InstrumentDefinition::FRA || rhs.type() == InstrumentDefinition::SWAP)) ||
                (lhs.type() == InstrumentDefinition::FRA && rhs.type() == InstrumentDefinition::SWAP))
            return true;
        else
            return false;
    }
    else
        return timeI1 < timeI2;
}

bool InstrumentDefinitionCompare::operator()(
        const InstrumentDefinition* ptrlhs,
        const InstrumentDefinition* ptrrhs) const
{
    return this->operator()(*ptrlhs, *ptrrhs);
}
//////////////////////////////////////////
// Definition of InstrumentDefinitionDurationCompare structure
//////////////////////////////////////////
bool InstrumentDefinitionDurationCompare::operator()(
        const InstrumentDefinition& lhs,
        const Duration& rhs) const
{
    int timeI1, timeI2;

    timeI1 = lhs.maturity().getDuration(Duration::DAY);
    timeI2 = rhs.getDuration(Duration::DAY);

    if(timeI1 == timeI2)
        return false;
    else
        return timeI1 < timeI2;
}

bool InstrumentDefinitionDurationCompare::operator()(
        const InstrumentDefinition *ptrlhs,
        const Duration *ptrrhs) const
{
    return this->operator()(
            *ptrlhs, *ptrrhs);
}
bool InstrumentDefinitionDurationCompare::operator()(
        const Duration& lhs,
        const InstrumentDefinition& rhs) const
{
    return !(this->operator()(rhs, lhs));
}

bool InstrumentDefinitionDurationCompare::operator()(
        const Duration *ptrrhs,
        const InstrumentDefinition *ptrlhs) const
{
    return !(this->operator()(*ptrrhs, *ptrlhs));
}

