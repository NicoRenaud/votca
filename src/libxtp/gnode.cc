/* 
 * Copyright 2009-2016 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "votca/xtp/gnode.h"
#include <votca/tools/property.h>
#include <boost/format.hpp>

using namespace std;

namespace votca {
    namespace xtp {
void GNode::AddDecayEvent(double _decayrate)
{
    GLink newEvent;
    newEvent.destination = -1;
    newEvent.rate = _decayrate;
    newEvent.initialrate = _decayrate;
    newEvent.dr = votca::tools::vec(0,0,0);
    newEvent.Jeff2 = 0.0;
    newEvent.decayevent=true;
    newEvent.reorg_out = 0.0;
    this->event.push_back(newEvent);
    hasdecay=true;
}

void GNode::AddEvent(int seg2, double rate12, votca::tools::vec dr, double Jeff2, double reorg_out)
{
    GLink newEvent;
    newEvent.destination = seg2;
    newEvent.rate = rate12;
    newEvent.initialrate = rate12;
    newEvent.dr = dr;
    newEvent.Jeff2 = Jeff2;
    newEvent.decayevent=false;
    newEvent.reorg_out = reorg_out;
    this->event.push_back(newEvent);
}


void GNode::InitEscapeRate()
{
    double newEscapeRate = 0.0;
    for(unsigned int i=0; i<this->event.size();i++)
    {
        newEscapeRate += this->event[i].rate;
    }
    this->escaperate = newEscapeRate;
    // cout << "Escape rate for segment " << this->id << " was set to " << newEscapeRate << endl;
}


        
    }
}