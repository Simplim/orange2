/*
    This file is part of Orange.

    Orange is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Orange is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Orange; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Authors: Janez Demsar, Blaz Zupan, 1996--2002
    Contact: janez.demsar@fri.uni-lj.si
*/


#include <math.h>

#include "stat.hpp"
#include "random.hpp"
#include "values.hpp"
#include "vars.hpp"
#include "errors.hpp"
#include "stladdon.hpp"
#include "domain.hpp"
#include "examplegen.hpp"
#include "examples.hpp"


#include "distvars.ppp"

DEFINE_TOrangeVector_classDescription(PDistribution, "TDistributionList")


#define CHECKVALTYPE(valType) \
 if (! (   (valType==TValue::INTVAR) && supportsDiscrete \
        || (valType==TValue::FLOATVAR) && supportsContinuous)) \
   raiseError("invalid value type");

#define NOT_IMPLEMENTED(x) { raiseError("'%s' is not implemented", x); throw 0; /*just to avoid warnings*/ }


TDistribution::TDistribution()
: unknowns(0.0),
  abs(0.0),
  cases(0.0),
  normalized(false),
  supportsDiscrete(false),
  supportsContinuous(false)
{}


TDistribution::TDistribution(PVariable var)
: variable(var),
  unknowns(0.0),
  abs(0.0),
  cases(0.0),
  normalized(false),
  supportsDiscrete(false),
  supportsContinuous(false)
{}


PDistribution TDistribution::create(PVariable var)
{ if (!var)
    return PDistribution();
  if (var->varType==TValue::INTVAR)
    return mlnew TDiscDistribution(var);
  if (var->varType==TValue::FLOATVAR)
    return mlnew TContDistribution(var);

  ::raiseErrorWho("Distribution", "unknown value type");
  return PDistribution(); // to make compiler happy
}



// General

float TDistribution::compatibility(const TSomeValue &) const
NOT_IMPLEMENTED("compatibility")

bool TDistribution::compatible(const TSomeValue &) const
NOT_IMPLEMENTED("compatible")

int TDistribution::compare(const TSomeValue &) const
NOT_IMPLEMENTED("compare")

TDistribution &TDistribution::operator += (const TDistribution &)
NOT_IMPLEMENTED("+=")

TDistribution &TDistribution::operator -= (const TDistribution &)
NOT_IMPLEMENTED("-=")

TDistribution &TDistribution::operator *= (const float &)
NOT_IMPLEMENTED("*=")


// Discrete 

const float &TDistribution::atint(const int &)
NOT_IMPLEMENTED("atint(int)")

const float &TDistribution::atint(const int &) const
NOT_IMPLEMENTED("atint(int)")

void TDistribution::addint(const int &, const float &)
NOT_IMPLEMENTED("addint(int, float)")

void TDistribution::setint(const int &, const float &)
NOT_IMPLEMENTED("add(int, float)")

int TDistribution::randomInt() const
NOT_IMPLEMENTED("randomInt()")

int TDistribution::highestProbIntIndex() const
NOT_IMPLEMENTED("highestProbIntIndex()")

int TDistribution::highestProbIntIndex(const long &) const
NOT_IMPLEMENTED("highestProbIntIndex(int)")

int TDistribution::highestProbIntIndex(const TExample &) const
NOT_IMPLEMENTED("highestProbIntIndex(TExample)")

float TDistribution::p(const int &) const
NOT_IMPLEMENTED("p(int)")

int TDistribution::noOfElements() const
NOT_IMPLEMENTED("noOfElements()")

// Continuous

const float &TDistribution::atfloat(const float &)
NOT_IMPLEMENTED("atfloat(float)")

const float &TDistribution::atfloat(const float &) const
NOT_IMPLEMENTED("atfloat(float)")

void TDistribution::addfloat(const float &, const float &)
NOT_IMPLEMENTED("addfloat(float, float)")

void TDistribution::setfloat(const float &, const float &)
NOT_IMPLEMENTED("add(float, float)")

float TDistribution::randomFloat() const
NOT_IMPLEMENTED("randomFloat()")

float TDistribution::highestProbFloatIndex() const
NOT_IMPLEMENTED("highestProbFloatIndex()")

float TDistribution::average() const
NOT_IMPLEMENTED("average()")

float TDistribution::dev() const
NOT_IMPLEMENTED("dev()")

float TDistribution::var() const
NOT_IMPLEMENTED("dev()")

float TDistribution::error() const
NOT_IMPLEMENTED("error()")

float TDistribution::percentile(const float &) const
NOT_IMPLEMENTED("percentile(float)")

float TDistribution::p(const float &) const
NOT_IMPLEMENTED("p(float)")




TDistribution &TDistribution::operator +=(PDistribution other)
{ return operator += (other.getReference()); }


TDistribution &TDistribution::operator -=(PDistribution other)
{ return operator -= (other.getReference()); }



float TDistribution::operator -  (const TSomeValue &v) const 
{ return 1-compatibility(v); }


float TDistribution::operator || (const TSomeValue &v) const
{ return 1-compatibility(v); }


const float &TDistribution::operator[](const TValue &val) const 
{ if (val.isSpecial()) {
    if (variable)
      raiseError("undefined value of attribute '%s'", variable->name.c_str());
    else
      raiseError("undefined attribute value");
  }
  CHECKVALTYPE(val.varType);
  return (val.varType==TValue::INTVAR) ? atint(int(val)) : atfloat(float(val));
}


const float &TDistribution::operator[](const TValue &val)
{ if (val.isSpecial()) {
    if (variable)
      raiseError("undefined value of attribute '%s'", variable->name.c_str());
    else
      raiseError("undefined attribute value");
  }
  CHECKVALTYPE(val.varType);
  return (val.varType==TValue::INTVAR) ? atint(int(val)) : atfloat(float(val));
}


void TDistribution::add(const TValue &val, const float &p)
{ if (val.isSpecial()) {
    unknowns += p;
    return;
  }

  CHECKVALTYPE(val.varType);
  if (val.varType==TValue::INTVAR)
    addint(val.intV, p);
  else
    addfloat(val.floatV, p);
}


void TDistribution::set(const TValue &val, const float &p)
{ if (!val.isSpecial()) {
    CHECKVALTYPE(val.varType);
    if (val.varType==TValue::INTVAR)
      setint(val.intV, p);
    else
      setfloat(val.floatV, p);
  }
}


TValue TDistribution::highestProbValue() const 
{ if (supportsDiscrete)
    return TValue(highestProbIntIndex());
  else if (supportsContinuous)
    return TValue(highestProbFloatIndex());
  else
    return TValue();
}


TValue TDistribution::highestProbValue(const long &random) const 
{ if (supportsDiscrete)
    return TValue(highestProbIntIndex(random));
  else if (supportsContinuous)
    return TValue(highestProbFloatIndex());
  else
    return TValue();
}


TValue TDistribution::highestProbValue(const TExample &exam) const 
{ if (supportsDiscrete)
    return TValue(highestProbIntIndex(exam));
  else if (supportsContinuous)
    return TValue(highestProbFloatIndex());
  else
    return TValue();
}


TValue TDistribution::randomValue() const
{ if (supportsDiscrete)
    return TValue(randomInt());
  else if (supportsContinuous)
    return TValue(randomFloat());
  else 
    return TValue();
}


float TDistribution::p(const TValue &val) const
{ if (val.isSpecial()) {
    if (variable)
      raiseError("undefined value of attribute '%s'", variable->name.c_str());
    else
      raiseError("undefined attribute value");
  }
  CHECKVALTYPE(val.varType);
  return (val.varType==TValue::INTVAR) ? p(int(val)) : p(float(val));
}




TDiscDistribution::TDiscDistribution() 
{ supportsDiscrete = true; };


TDiscDistribution::TDiscDistribution(PVariable var) 
: TDistribution(var), 
  distribution(vector<float>(var->noOfValues(), 0.0))
{ if (var->varType!=TValue::INTVAR)
     raiseError("attribute '%s' is not discrete", var->name.c_str());
  supportsDiscrete = true;
}


TDiscDistribution::TDiscDistribution(int values, float value) 
: distribution(vector<float>(values, value))
{ cases = abs = value*values;
  supportsDiscrete = true;
}


TDiscDistribution::TDiscDistribution(const vector<float> &f) 
: distribution(f)
{ abs = 0.0;
  for (const_iterator fi(begin()), fe(end()); fi!=fe; abs += *(fi++));
  cases = abs;
  supportsDiscrete = true;
}


TDiscDistribution::TDiscDistribution(const float *f, const int &len)
: distribution(f, f+len)
{ abs = 0.0;
  for (const_iterator fi(begin()), fe(end()); fi!=fe; abs += *(fi++));
  cases = abs;
  supportsDiscrete = true;
}


TDiscDistribution::TDiscDistribution(PDistribution other) 
: TDistribution(other.getReference())
{ supportsDiscrete = true; }


TDiscDistribution::TDiscDistribution(PDiscDistribution other) 
: TDistribution(other.getReference())
{ supportsDiscrete = true; }


const float &TDiscDistribution::atint(const int &v)
{ int ms = v + 1 - size();
  if (ms>0) {
    reserve(v+1);
    while (ms--)
      push_back(0.0);
  }
  return distribution[v]; 
}


const float &TDiscDistribution::atint(const int &v) const
{ if (!size())
    raiseError("empty distribution");
  if ((v < 0) || (v >= int(size()))) 
    raiseError("value %s out of range 0-%s", v, size()-1);
  return at(v); 
}


void TDiscDistribution::addint(const int &v, const float &w)
{ if ((v<0) || (v>1e6))
    raiseError("invalid value");

  int ms = v+1 - size();
  if (ms>0) {
    reserve(v+1);
    while (ms--)
      push_back(0.0);
  }

  float &val = distribution[v];
  val += w;
  abs += w;
  cases += w;
  normalized = false;
}


void TDiscDistribution::setint(const int &v, const float &w)
{ if ((v<0) || (v>1e6))
    raiseError("invalid value");

  int ms = v+1 - size();
  if (ms>0) {
    reserve(v+1);
    while (ms--)
      push_back(0.0);
  }

  float &val=distribution[v];
  abs += w-val;
  cases += w-val;
  val = w;
  normalized = false;
}


TDistribution &TDiscDistribution::operator +=(const TDistribution &other)
{
  const TDiscDistribution *mother=dynamic_cast<const TDiscDistribution *>(&other);
  if (!mother)
    raiseError("wrong type of distribution for +=");

  int ms = mother->size() - size();
  if (ms>0) {
    reserve(mother->size());
    while (ms--)
      push_back(0.0);
  }
  
  iterator ti = begin();
  const_iterator oi = mother->begin(), oe = mother->end();
  while(oi!=oe)
    *(ti++) += *(oi++);
  abs += mother->abs;
  cases += mother->cases;
  unknowns += mother->unknowns;
  normalized = false;
  return *this;
}


TDistribution &TDiscDistribution::operator -=(const TDistribution &other)
{
  const TDiscDistribution *mother=dynamic_cast<const TDiscDistribution *>(&other);
  if (!mother)
    raiseError("wrong type of distribution for -=");

  int ms = mother->size() - size();
  if (ms>0) {
    reserve(mother->size());
    while (ms--)
      push_back(0.0);
  }
  
  iterator ti = begin();
  const_iterator oi = mother->begin(), oe = mother->end();
  while(oi!=oe)
    *(ti++) -= *(oi++);
  abs -= mother->abs;
  cases -= mother->cases;
  unknowns -= mother->unknowns;
  normalized = false;
  return *this;
}


TDistribution &TDiscDistribution::operator +=(PDistribution other)
{ return operator += (other.getReference()); }


TDistribution &TDiscDistribution::operator -=(PDistribution other)
{ return operator -= (other.getReference()); }



TDistribution &TDiscDistribution::operator *=(const float &weight)
{ for(iterator di(begin()); di!=end(); (*(di++)) *= weight);
  abs *= weight;
  normalized = false;
  return *this;
}


TDistribution &TDiscDistribution::operator *=(const TDistribution &other)
{ const TDiscDistribution *mother=dynamic_cast<const TDiscDistribution *>(&other);
  if (!mother)
    raiseError("wrong type of distribution for *=");

  abs = 0.0;
  iterator di = begin(), de = end();
  const_iterator di2 = mother->begin(), de2 = mother->end();
  while ((di!=de) && (di2!=de2))
    abs += (*(di++) *= *(di2++));

  if (di!=de)
    erase(di, de);

  normalized = false;
  return *this;
}


TDistribution &TDiscDistribution::operator *= (PDistribution other)
{ return operator *= (other.getReference()); }


TDistribution &TDiscDistribution::operator /=(const TDistribution &other)
{ const TDiscDistribution *mother=dynamic_cast<const TDiscDistribution *>(&other);
  if (!mother)
    raiseError("wrong type of distribution for /=");

  abs = 0.0;
  iterator di = begin(), de = end();
  const_iterator di2 = mother->begin(), de2 = mother->end();
  for (; (di!=de) && (di2!=de2); di++, di2++) {
    if ((-1e-20 < *di2) && (*di2 < 1e-20)) {
      if ((*di<-1e-20) || (*di>1e-20))
        raiseError("division by zero in /=");
    }
    else
      abs += (*di /= *di2);
  }

  if (di!=de)
    erase(di, de);

  normalized = false;
  return *this;
}


TDistribution &TDiscDistribution::operator /= (PDistribution other)
{ return operator /= (other.getReference()); }


TDistribution &TDiscDistribution::mul(const TDistribution &other, const float &weight)
{ const TDiscDistribution *mother=dynamic_cast<const TDiscDistribution *>(&other);
  if (!mother)
    raiseError("wrong type of distribution for -=");

  abs = 0.0;
  iterator di = begin(), de = end();
  const_iterator di2 = mother->begin(), de2 = mother->end();
  while ((di!=de) && (di2!=de2))
    abs += (*(di++) *= weight * *(di2++));

  if (di!=de)
    erase(di, de);

  normalized = false;
  return *this;
}


TDistribution &TDiscDistribution::mul(PDistribution other, const float &weight)
{ return mul(other.getReference(), weight); }


/*  Returns normalized scalar products of distributions of 'other' and 'this'.
    The result corresponds to a probability that two random values chosen according
    to the given distributions are same. */
float TDiscDistribution::compatibility(const TSomeValue &ot) const
{ const TDiscDistribution *dv=dynamic_cast<const TDiscDistribution *>(&ot);
  if (dv) {
    float sum=0;
    for(const_iterator i1=begin(), i2=dv->begin();
        (i1!=end());
        sum += *(i1++) * *(i2++))
    return sum/abs/dv->abs;
  }

  const TValue *vv=dynamic_cast<const TValue *>(&ot);
  if (   (vv) 
      || (vv->varType==TValue::INTVAR))
    return (vv->intV>int(size())) ? 0.0 : operator[](vv->intV)/abs;
      
  raiseError("can't compare values of different types");
  return 0.0; // to make compilers happy
}


/*  Declared only since it is abstract in TSomeValue.
    Definition is somewhat artificial: compare does a lexicographical comparison of probabilities. */
int  TDiscDistribution::compare(const TSomeValue &ot) const
{ const TDiscDistribution *dv=dynamic_cast<const TDiscDistribution *>(&ot);
  if (!dv)
    raiseError("can't compare values of different types");

  const_iterator i1=begin(), i2=dv->begin();
  for( ; (i1!=end()) && (*i1==*i2); i1++, i2++);
  if (i1==end())
    return 0;
  else 
    if (*i1<*i2)
      return -1;
  return 1;
}


/*  Declared only since it is abstract in TSomeValue.
    Definitions is somewhat artificial: compatible returns true if compatibility>0 (i.e. if there
    is a non-xero probability that a random values with given distributions are same). */
bool  TDiscDistribution::compatible (const TSomeValue &ot) const
{ return (compatibility(ot)>0); }


void TDiscDistribution::normalize()
{ if (!normalized) {
    if (abs) {
      this_ITERATE(dvi)
        *dvi /= abs;
      abs=1.0;
    }
    else 
      if (size()) {
        float p = 1.0/float(size());
        this_ITERATE(dvi)
          *dvi = p;
        abs = 1.0;
      }
   normalized = true;
  }
}


int TDiscDistribution::highestProbIntIndex() const
{
  if (!size())
    return 0;

  int wins = 1;
  int best = 0;
  float bestP = operator[](0);
  for(int i = 1, e = int(size()); --e; i++)
    if (operator[](i) > bestP) {
      best = i;
      bestP = operator[](i);
      wins = 1;
    }
    else if ((operator[](i)==bestP) && _globalRandom->randbool(++wins))
      best = i;

  return best;
}


int TDiscDistribution::highestProbIntIndex(const long &random) const
{
  if (!size())
    return 0;

  int wins = 1;
  int best = 0;
  float bestP = operator[](0);
  int i, e;

  for(i = 1, e = int(size()); --e; i++)
    if (operator[](i) > bestP) {
      best = i;
      bestP = operator[](i);
      wins = 1;
    }
    else if (operator[](i)==bestP)
      wins++;

  if (wins==1)
    return best;

  for(i = 0, wins = 1 + random % wins; wins; i++)
    if (operator[](i)==bestP)
      wins--;

  return i-1;
}


int TDiscDistribution::highestProbIntIndex(const TExample &exam) const
{
  if (!size())
    return 0;

  int wins = 1;
  int best = 0;
  float bestP = operator[](0);
  int i, e;

  for(i = 1, e = int(size()); --e; i++)
    if (operator[](i) > bestP) {
      best = i;
      bestP = operator[](i);
      wins = 1;
    }
    else if (operator[](i)==bestP)
      wins++;

  if (wins==1)
    return best;

  int sumex = exam.sumValues();
  wins = 1 + (sumex ? sumex : _globalRandom->randlong()) % wins;

  i = 0;    
  while (wins)
    if (operator[](i++)==bestP)
      wins--;

  return i-1;
}


float TDiscDistribution::highestProb() const
{
  float best=-1;
  for(int i=0, isize = size(); i<isize; i++)
    if (operator[](i) > best)
      best=i;
  if (best>=0)
    return operator[](best);
  else
    return size() ? 1.0/size() : 0.0;
}


bool TDiscDistribution::noDeviation() const
{ const_this_ITERATE(dvi)
    if (*dvi)
      return *dvi == abs;
  return size()==1;
}
  

int TDiscDistribution::randomInt() const
{ float ri = _globalRandom->randfloat(abs);
  const_iterator di(begin());
  while (ri > *di)
    ri -= *(di++);
  return int(di-begin());
}


float TDiscDistribution::p(const int &x) const
{ if (!abs) 
    return size() ? 1.0/size() : 0.0;
  return atint(x)/abs; 
}

int TDiscDistribution::noOfElements() const
{ return size(); }



TContDistribution::TContDistribution()
: sum(0.0),
  sum2(0.0)
{ supportsContinuous = true; }


TContDistribution::TContDistribution(const map<float, float> &dist)
: distribution(dist), 
  sum(0.0),
  sum2(0.0)
{ abs = 0.0;
  this_ITERATE(di) {
    abs+=(*di).second;
    sum+=(*di).second*(*di).first;
    sum2+=(*di).second*(*di).first*(*di).first;
  }
  cases = abs;
  supportsContinuous = true; 
}


TContDistribution::TContDistribution(PVariable var) 
: TDistribution(var),
  sum(0.0),
  sum2(0.0)
{ if (var->varType!=TValue::FLOATVAR)
     raiseError("attribute '%s' is not continuous", var->name.c_str());
  supportsContinuous = true; 
}


const float &TContDistribution::atfloat(const float &v)
{ if (find(v)!=end())
    distribution[v]=0;
  return distribution[v]; 
}


const float &TContDistribution::atfloat(const float &v) const
{ const_iterator vi=find(v);
  if (vi==end())
    raiseError("value %5.3f does not exist", v);
  return (*vi).second;
}


void TContDistribution::addfloat(const float &v, const float &w)
{ 
  iterator vi=find(v);
  if (vi==end())
    distribution[v]=w;
  else
    (*vi).second+=w;

  abs += w;
  cases += w;
  sum += w * v;
  sum2 += w * v*v;
  normalized = false;
}


void TContDistribution::setfloat(const float &v, const float &w)
{ 
  iterator vi=find(v);
  if (vi==end()) {
    distribution[v]=w;
    abs += w;
    cases += w;
    sum += w * v;
    sum += w * v*v;
  }
  else {
    float dif = w - (*vi).second;
    abs += dif;
    cases += w;
    sum += dif * v;
    sum2 += dif * v*v;
    (*vi).second += w;
  }
 
  normalized = false;
}


TDistribution &TContDistribution::operator +=(const TDistribution &other)
{
  const TContDistribution *mother = dynamic_cast<const TContDistribution *>(&other);
  if (!mother)
    raiseError("wrong distribution type for +=");

  const_PITERATE(TContDistribution, oi, mother) 
    addfloat((*oi).first, (*oi).second);

  unknowns += mother->unknowns;

  return *this;
}


TDistribution &TContDistribution::operator -=(const TDistribution &other)
{
  const TContDistribution *mother = dynamic_cast<const TContDistribution *>(&other);
  if (!mother)
    raiseError("wrong distribution type for -=");

  const_PITERATE(TContDistribution, oi, mother) 
    addfloat((*oi).first, -(*oi).second);

  unknowns -= mother->unknowns;

  return *this;
}


TDistribution &TContDistribution::operator +=(PDistribution other)
{ return operator += (other.getReference()); }


TDistribution &TContDistribution::operator -=(PDistribution other)
{ return operator -= (other.getReference()); }



TDistribution &TContDistribution::operator *=(const float &weight)
{ for(iterator i(begin()), e(end()); i!=e; (*(i++)).second*=weight);
  abs *= weight;
  sum *= weight;
  sum2 *= weight;
  normalized = false;
  return *this;
}


float TContDistribution::highestProbFloatIndex() const
{
  int wins=0;
  const_iterator best=0;
  const_this_ITERATE(i)
    if (   (wins==0) && ((wins=1)==1)
        || ((*i).second >  (*best).second) && ((wins=1)==1)
        || ((*i).second == (*best).second) && globalRandom->randbool(++wins))
      best = i;
  return (*best).first;
}


float TContDistribution::highestProb() const
{
  int wins=0;
  const_iterator best=0;
  const_this_ITERATE(i)
    if (   (wins==0) && ((wins=1)==1)
        || ((*i).second >  (*best).second) && ((wins=1)==1)
        || ((*i).second == (*best).second) && globalRandom->randbool(++wins))
      best = i;

  if (wins)
    return (*best).second;
  else
    return size() ? 1.0/size() : 0.0;
}


bool TContDistribution::noDeviation() const
{ return size()==1;
}


float TContDistribution::average() const
{ if (!abs)
    raiseError("cannot compute average (no values)");
  return sum/abs ; 
}


float TContDistribution::dev() const
{ if (abs<=2.0)
    raiseError("cannot compute standard deviation (no values)");
  return sqrt((sum2-sum*sum/abs)/abs);
}
  
float TContDistribution::var() const
{ if (!abs)
    raiseError("cannot compute variance (no values)");
  return (sum2-sum*sum/abs)/abs;
}
  
float TContDistribution::error() const
{ return abs<=1.0 ? 0.0 : sqrt((sum2-sum*sum/abs)/(abs-1) / abs); }


float TContDistribution::percentile(const float &perc) const
{ if ((perc<0) || (perc>99))
    raiseError("invalid percentile");

  float togo = abs*perc/100.0;
  const_iterator ths(begin()), prev, ee(end());

  if (ths == ee)
    raiseError("empty distribution");

  while ((ths != ee) && (togo > 0)) {
    togo -= (*prev).first;
    prev = ths;
    ths++;
  }

  if ((togo < 0) || (ths == ee))
    return (*prev).first;

  // togo==0.0 && ths!=ee
  return ((*prev).first + (*ths).first) / 2.0;
}


void TContDistribution::normalize()
{ if (!normalized) {
    if (abs) {
      this_ITERATE(dvi)
        (*dvi).second /= abs;
      sum /= abs;
      sum2 /= abs;
      abs = 1.0;
    }
    else if (size()) {
      float p = 1.0/float(size());
      sum = 0.0;
      sum2 = 0.0;
      this_ITERATE(dvi) {
        (*dvi).second = p;
        sum += (*dvi).first;
        sum2 += sqr((*dvi).first);
      }
      sum /= abs;
      sum2 /= abs;
      abs = 1.0;
    }

    normalized = true;
  }
}


float TContDistribution::randomFloat() const
{ float ri = _globalRandom->randfloat(abs);
  const_iterator di(begin());
  while (ri > (*di).first)
    ri -= (*(di++)).first;
  return (*di).second;
}

float TContDistribution::p(const float &x) const
{ const_iterator rb = upper_bound(x);
  if (rb==end())
    return 0.0;
  if ((*rb).first==x)
    return (*rb).second;
  if (rb==begin())
    return 0.0;
  const_iterator lb = rb;
  lb--;

  return (*lb).second + (x - (*lb).first) * ((*rb).second - (*lb).second) / ((*rb).first - (*lb).first);
}



TGaussianDistribution::TGaussianDistribution(const float &amean, const float &asigma)
: mean(amean),
  sigma(asigma)
{}


TGaussianDistribution::TGaussianDistribution(PDistribution dist)
: mean(dist->average()),
  sigma(sqrt(dist->error()))
{ normalized = true; }



float TGaussianDistribution::average() const
{ return mean; }


float TGaussianDistribution::var() const
{ return sigma*sigma; }
  

float TGaussianDistribution::dev() const
{ return sigma; }
  

float TGaussianDistribution::error() const
{ return sigma; }
  

void TGaussianDistribution::normalize()
{}


float TGaussianDistribution::highestProbFloatIndex() const
{ return mean; }


#define pi 3.1415926535897931

float TGaussianDistribution::highestProb() const
{ return 1/(sigma * sqrt(2*pi)); }


float TGaussianDistribution::randomFloat() const
{ return (float)gasdev((double)mean, (double)sigma, *_globalRandom); }


float TGaussianDistribution::p(const float &x) const
{ return exp(-sqr((x-mean)/sigma)) / (sigma*sqrt(2*pi)); }


bool TGaussianDistribution::noDeviation() const
{ return sigma==0.0; }


TDomainDistributions::TDomainDistributions()
{}


TDomainDistributions::TDomainDistributions(PExampleGenerator gen, const long weightID)
{ reserve(gen->domain->variables->size());
  PITERATE(TVarList, vi, gen->domain->variables)
    push_back(TDistribution::create(*vi));

  for(TExampleIterator fi(gen->begin()); fi; ++fi) {
    TExample::iterator ei=(*fi).begin();
    float weight=WEIGHT(*fi);
    for(iterator di=begin(); di!=end(); di++, ei++)
      (*di)->add(*ei, weight);
  }
}


void TDomainDistributions::normalize()
{ this_ITERATE(di)
    (*di)->normalize(); 
}


PDistribution getClassDistribution(PExampleGenerator gen, const long &weightID)
{ if (!gen)
    raiseErrorWho("getClassDistribution", "no examples");

  if (!gen->domain || !gen->domain->classVar)
    raiseErrorWho("getClassDistribution", "invalid example generator or class-less domain");

  PDistribution classDist = TDistribution::create(gen->domain->classVar);
  TDistribution *uclassdist = const_cast<TDistribution *>(classDist.getUnwrappedPtr());
  PEITERATE(ei, gen)
    uclassdist->add((*ei).getClass(), WEIGHT(*ei));
  return classDist;
}

#undef NOT_IMPLEMENTED
#undef CHECKVALTYPE
