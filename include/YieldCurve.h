#ifndef _INCLUDE_YIELDCURVE_H_
#define _INCLUDE_YIELDCURVE_H_

#include <vector>
#include <map>
#include <utility>
#include <cmath>
#include <functional>
#include <stdexcept>

#include "Instrument.h"
class Date; // Forward Declaration of Date class in "Date.h"
struct DateCompare;
//class InstrumentDefinition; // Forward Declaration of InstrumentDefinition class in "Instrument.h"
//class InstrumentValues; // Forwaed Declaration of InstrumentValues class in "Instrument.h"

typedef std::pair<Date, double> CurveDataType;

class YieldCurveInstance;

class YieldCurveDefinition
{
    public:
        YieldCurveDefinition(
                std::vector<InstrumentDefinition *>& instrDefs,
                double compoundFreq);
        ~YieldCurveDefinition();

        // Section about Instrument Definitions
        std::vector<InstrumentDefinition *> getAllDefinitions() const;

        // Section about Instrument Values
        enum CURVETYPE {ZEROCOUPONRATE};
        YieldCurveInstance* bindData(InstrumentValues *instrVals,
                YieldCurveDefinition::CURVETYPE type);

        InstrumentDefinition *getDefinitionByID(int id);
    protected:
        void _insertFakeInstrumentDefs();

        // map from the instrument id to the vector index
        // of the instrument definition
        std::map<int, int> _instrDefIndicesMap;

        // FIX: we need a way to better manage the pointers
        // maybe by using shared_ptr
        std::vector<InstrumentDefinition *> _instrDefs;
        double _compoundFreq;
};

struct CurvePointDesc
{
    CurvePointDesc(Date& idate, double ivalue, double ideltaT,
            enum InstrumentDefinition::TYPE iInstrType):
        date(idate), value(ivalue), deltaT(ideltaT),
        instrType(iInstrType){};

    bool operator<(const struct CurvePointDesc& rhs) const;

    Date date;
    double value;
    double deltaT;
    enum InstrumentDefinition::TYPE instrType;
};
typedef struct CurvePointDesc CurvePoint_t;

class YieldCurveInstance
{
    public:
        explicit YieldCurveInstance(const YieldCurveInstance& rhs);
        virtual YieldCurveInstance& operator=(const YieldCurveInstance& rhs);

        virtual ~YieldCurveInstance(){};

        // return the start Date of the curve
        inline Date startDate() const {return _startDate;};

        // insert the data to the curve
        void insert(CurvePointDesc& data);

        // This return the actually point value on the curve
        double operator[](Date& date) const;
        double getDf(Date& date) const;
    protected:
        explicit YieldCurveInstance(const Date& startDate);

        // void insert() will call this when it
        // inserts the df to the curve, and
        // this function will change the df to
        // the corresponding value
        virtual double _convertDfToSpecific(double df, double deltaT) const = 0;
        virtual double _convertSpecificToDf(double specVal, double deltaT) const = 0; 


        // Store the Points on the curve
        std::vector<CurvePoint_t> _curveData;

        // FIX: It seems we do not need this
        // map from the Date to the vector index of 
        // _curveDate
        std::map<Date, int> _curveDataIndicesMap;
        Date _startDate;
};

class ZeroCouponRateCurve:
    public YieldCurveInstance
{
    public:
        friend YieldCurveInstance* YieldCurveDefinition::bindData(
                InstrumentValues*,
                YieldCurveDefinition::CURVETYPE);

        explicit ZeroCouponRateCurve(const ZeroCouponRateCurve& rhs);
        virtual YieldCurveInstance& operator=(YieldCurveInstance& rhs);
        ZeroCouponRateCurve& operator=(ZeroCouponRateCurve& rhs);

        virtual ~ZeroCouponRateCurve();

    private:
        ZeroCouponRateCurve(double compoundFreq, const Date& startDate);

        // Section about Curve Data
        inline double _dfToZ(double df, double deltaT) const
        {
            return _compoundFreq * (
                    exp(-log(df) / (_compoundFreq * deltaT)) - 1.0f);
        };
        inline double _ZTodf(double Z, double deltaT) const
        {
            return exp(-1.0f * deltaT * _compoundFreq *
                    log(1.0f + Z / _compoundFreq));
        };

        virtual double _convertDfToSpecific(double df, double deltaT) const
            {return _dfToZ(df, deltaT);};

        virtual double _convertSpecificToDf(double specVal, double deltaT) const 
            {return _ZTodf(specVal, deltaT);};

        double _compoundFreq;
};

// calculate the compound rate of specified Instrument
// type and the given Date from the yield curve
double getCompoundRate(YieldCurveInstance&, Date&,
        InstrumentDefinition::TYPE, double);

class YieldCurveException:
    public std::runtime_error
{
    public:
        YieldCurveException(std::string& message):
            std::runtime_error(message){};
};

#endif //_INCLUDE_YIELDCURVE_H_
