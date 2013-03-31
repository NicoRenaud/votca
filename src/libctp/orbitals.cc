/*
 *            Copyright 2009-2012 The VOTCA Development Team
 *                       (http://www.votca.org)
 *
 *      Licensed under the Apache License, Version 2.0 (the "License")
 *
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "votca/ctp/orbitals.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <iterator>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/vector.hpp>

using namespace std;

namespace votca { namespace ctp {

Orbitals::Orbitals() { 
    
    _basis_set_size = 0;
    _occupied_levels = 0;
    _unoccupied_levels = 0;
    _number_of_electrons = 0;
    
    _has_basis_set_size = false;
    _has_occupied_levels = false;
    _has_unoccupied_levels = false;
    _has_number_of_electrons = false;   
    _has_level_degeneracy = false;
    _has_mo_coefficients = false;
    _has_mo_energies = false;
    _has_overlap = false;
    
    _save_mo_coefficients = true;
    _save_overlap = true;
        
};   
    
Orbitals::~Orbitals() { 
    _mo_energies.clear();
    _mo_coefficients.clear();
    _overlap.clear();
};   

const int    &Orbitals::getBasisSetSize() const { 
    if ( _has_basis_set_size ) {
        return _basis_set_size; 
    } else {
        throw std::runtime_error(" Basis set size is unknown. Parse a log file first. " );
    }
}

void          Orbitals::setBasisSetSize( int &basis_set_size ){
    _has_basis_set_size = true; 
    _basis_set_size = basis_set_size; 
}


int     Orbitals::getNumberOfLevels() {
    if ( _has_occupied_levels && _has_unoccupied_levels ) {
        return  _occupied_levels + _unoccupied_levels; 
    } else {
        throw std::runtime_error(" Number of levels is unknown. Parse a log file first. " );
    }
}

void          Orbitals::setNumberOfLevels( int &occupied_levels, int &unoccupied_levels  ){
    _has_occupied_levels = true; 
    _has_unoccupied_levels = true; 
    _occupied_levels = occupied_levels; 
    _unoccupied_levels = unoccupied_levels; 
}

const int     &Orbitals::getNumberOfElectrons() const {
    if ( _has_number_of_electrons ) {
        return  _number_of_electrons; 
    } else {
        throw std::runtime_error(" Number of electrons is unknown. Parse a log file first. " );
    }
}

void          Orbitals::setNumberOfElectrons( int &electrons ){
    _has_number_of_electrons = true; 
    _number_of_electrons = electrons; 
}

/**
 * 
 * @param _energy_difference [ev] Two levels are degenerate if their energy is smaller than this value
 * @return A map with key as a level and a vector which is a list of close lying orbitals
 */
bool Orbitals::CheckDegeneracy( double _energy_difference ) {
    
    ub::vector<double>::iterator it1 = _mo_energies.begin();
    bool _degenerate = false;
    bool _verbose = true;
    
    if ( _verbose ) cout << endl <<"... ... Checking level degeneracy " << endl;
    
    _level_degeneracy.clear();
            
    while ( it1 !=_mo_energies.end() ) {
        
        // in all containers counters start with 0; real life - with 1
        int _level1 = std::distance(_mo_energies.begin(), it1) + 1;
        
        // add the level itself - it is easier to loo over all levels later
        _level_degeneracy[_level1].push_back( _level1 );        
        
        ub::vector<double>::iterator it2 = it1;
        it2++;
        
        while (  it2 !=_mo_energies.end() ) {
            //cout << _level1 << ":" << *it1 << ":" << *it2 << endl;
            double energy1 = *it1;
            double energy2 = *it2;
            
            // in all containers counters start with 0; real life - with 1
            int _level2 = std::distance(_mo_energies.begin(), it2) + 1;
            
            if ( abs(energy1 - energy2)*_conv_Hrt_eV < _energy_difference ) {
                _level_degeneracy[_level1].push_back( _level2 );
                _level_degeneracy[_level2].push_back( _level1 );
                _degenerate = true;
            }
            it2++;
        }
        it1++;
    }

    if ( _verbose ){ 

        if ( _degenerate ) {
            cout << "... ... Some levels are degenerate" << endl; 
            for (std::map<int, std::vector<int> >::iterator it = _level_degeneracy.begin();
                    it != _level_degeneracy.end();
                    ++it) {
                // output only degenerate levels
                if ( (it->second).size() > 1 ) {
                    std::cout << "... ... level  "<< it->first << " : ";
                    for (vector<int>::iterator lev = (it->second).begin(); lev != (it->second).end(); lev++)
                            cout << *lev << " ";
                    cout << endl;
                }
            }
        } else {
            cout << "... ... No degeneracy found" << endl;  
        }

        cout << "... ... Done checking level degeneracy" << endl;   
    
    }
    
    _has_level_degeneracy = true;
    return _degenerate;
    
}    

std::vector<int>* Orbitals::getDegeneracy( int level, double _energy_difference ) {
    if ( !_has_level_degeneracy ) {
        
        CheckDegeneracy( _energy_difference );       
        /* 

        int _ld = _level_degeneracy.at(level).size();       
        if ( _ld > 1 ) {
                cout << "....level " << level << " degeneracy is: " <<  _ld << endl;
        } else {
                cout << "....level " << level << " is not degenerate" << endl;
        }
        */
        
    }        
    
    return &_level_degeneracy.at(level);
}


}}
