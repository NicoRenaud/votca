/*
 *            Copyright 2009-2016 The VOTCA Development Team
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

#ifndef _VOTCA_XTP_DFTENGINE_H
#define	_VOTCA_XTP_DFTENGINE_H

#include <votca/xtp/segment.h>
#include <votca/xtp/orbitals.h>

#include <votca/xtp/logger.h>



#include <boost/filesystem.hpp>
#include <votca/xtp/ERIs.h>
#include <votca/xtp/diis.h>
#include <votca/xtp/mixing.h>
#include <votca/xtp/numerical_integrations.h>

namespace votca { namespace xtp {
    namespace ub = boost::numeric::ublas;

        /**
         * \brief Electronic ground-state via Density-Functional Theory
         *
         * Evaluates electronic ground state in molecular systems based on
         * density functional theory with Gaussian Orbitals.
         * 
         */

class DFTENGINE 
{
public:

    DFTENGINE() {_addexternalsites=false;
            };
   ~DFTENGINE(){
    
   };

   
   
    void    Initialize( Property *options);
    std::string  Identify() { return "dftengine"; }
   
    void    CleanUp();

    void setLogger( Logger* pLog ) { _pLog = pLog; }
    
    void setExternalcharges(std::vector<APolarSite*> externalsites){
        _externalsites=externalsites;
        _addexternalsites=true;
    }
    
    void setExternalGrid(std::vector<double> electrongrid,std::vector<double> nucleigrid){
        _externalgrid=electrongrid;
       _externalgrid_nuc=nucleigrid;
    }
    
 
    
    bool Evaluate(   Orbitals* _orbitals );

    // interfaces for options getting/setting
    //bool get_do_qp_diag(){ return _do_qp_diag ;}
    //void set_do_qp_diag( bool inp ){ _do_qp_diag = inp;}
    
    
    private:

    Logger *_pLog;
    
    void Prepare( Orbitals* _orbitals );
    void SetupInvariantMatrices();
    ub::matrix<double> AtomicGuess(Orbitals* _orbitals);
    ub::matrix<double> DensityMatrix_frac( const ub::matrix<double>& _MOs,const ub::vector<double>& MOenergies, ub::vector<double>& occupation, int numofelec );
    string Choosesmallgrid(string largegrid);
    void NuclearRepulsion();
    double ExternalRepulsion();
     double ExternalGridRepulsion(std::vector<double> externalpotential_nuc);
   
    //bool   _maverick;
    
    // program tasks
    //bool                                _do_qp_diag;
    
    // storage tasks
    //bool                                _store_qp_pert;
    
    int                                 _openmp_threads;
    
    
    std::string _outParent;
    std::string _outMonDir;
    
    // options
    std::string _dft_options;
    Property _dftengine_options; 
    
    // atoms
    std::vector<QMAtom*>                _atoms;

    // basis sets
    std::string                              _auxbasis_name;
    std::string                              _dftbasis_name;
    std::string                              _ecp_name;
    BasisSet                            _dftbasisset;
    BasisSet                            _auxbasisset;
    BasisSet                            _ecpbasisset;
    AOBasis                             _dftbasis;
    AOBasis                             _auxbasis;
    AOBasis                             _ecp;
    
    bool                                _with_ecp;
    bool                                _with_RI;
    string                              _4cmethod;
    
    // numerical integration Vxc
    std::string                              _grid_name;
    std::string                             _grid_name_small;
    bool                                _use_small_grid;
    NumericalIntegration                _gridIntegration;
    NumericalIntegration                 _gridIntegration_small;
    //used to store Vxc after final iteration
  
    //numerical integration externalfield;
    //this will not remain here but be moved to qmape
    bool                                    _do_externalfield;
    std::string                              _grid_name_ext;
    NumericalIntegration                _gridIntegration_ext;
    std::vector<double>                     _externalgrid;
    std::vector<double>                     _externalgrid_nuc;
    

    // AO Matrices
    AOOverlap                           _dftAOoverlap;
   
   // AOCoulomb                           _dftAOcoulomb;
    
    ub::matrix<double>                  _AuxAOcoulomb_inv;
    ub::matrix<double>                  _dftAOdmat;
    AOKinetic                           _dftAOkinetic;
    AOESP                               _dftAOESP;
    AOECP                               _dftAOECP;
    
    AODipole_Potential                  _dftAODipole_Potential;
    AOQuadrupole_Potential              _dftAOQuadrupole_Potential;
    bool                                _with_guess;
    string                              _initial_guess;
    double                              E_nucnuc;
    
    // COnvergence 
    double                              _mixingparameter;
    double                              _Econverged;
    double                              _error_converged;
    int                            _numofelectrons;
    int                                 _max_iter;
    int                                 _this_iter;
    
    
    //DIIS variables
    Diis                               _diis;
    bool                              _usediis;
    unsigned                          _histlength;
    bool                              _maxout;
    string                            _diismethod;
    ub::matrix<double>                _Sminusonehalf;
    double                              _diis_start;                 
    bool                        _useautomaticmixing;
    //Electron repulsion integrals
    ERIs                                _ERIs;
    
    // external charges
     std::vector<APolarSite*>        _externalsites;
     bool                            _addexternalsites;
    
    // exchange and correlation
    std::string                              _xc_functional_name;


  
    
};


}}

#endif	/* _VOTCA_XTP_DFTENGINE_H */
