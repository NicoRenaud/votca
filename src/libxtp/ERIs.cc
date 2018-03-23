/* 
 *            Copyright 2009-2017 The VOTCA Development Team
 *                       (http://www.votca.org)
 *
 *      Licensed under the Apache License, Version 2.0 (the "License")
 *
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <votca/xtp/ERIs.h>



using namespace votca::tools;

namespace votca {
    namespace xtp {
        namespace ub = boost::numeric::ublas;
        
        
        
    void ERIs::Initialize(AOBasis &_dftbasis, AOBasis &_auxbasis,const ub::matrix<double> &inverse_Coulomb) {

           _inverse_Coulomb=inverse_Coulomb;
           
            _threecenter.Fill( _auxbasis, _dftbasis );
          

            return;
        }



        void ERIs::Initialize_4c_small_molecule(AOBasis &_dftbasis) {

          _fourcenter.Fill_4c_small_molecule( _dftbasis );

          return;
        }

        
        
        void ERIs::CalculateERIs (const ub::matrix<double> &DMAT){

            //cout << _auxAOcoulomb.Matrix()<<endl;
            //cout << "inverse Coulomb"<< endl;
            //cout << _inverse_Coulomb<<endl;
            
            _ERIs=ub::zero_matrix<double>(DMAT.size1());
           
            const ub::symmetric_matrix<double> dmat_symm=DMAT;
        
            const ub::vector<double>& dmatasarray=DMAT.data();
          
            ub::matrix<double> Itilde=ub::matrix<double>(_threecenter.getSize(),1);
            //cout << _threecenter.getSize() << " Size-Threecenter"<<endl;
            //check Efficiency !!!! someday 
            #pragma omp parallel for
            for ( int _i=0; _i<_threecenter.getSize();_i++){
                const ub::symmetric_matrix<double> &threecenter=_threecenter.getDatamatrix(_i);
                // Trace over prod::DMAT,I(l)=componentwise product over 
                double trace=0;
                for ( unsigned _j=0; _j<dmat_symm.size1();_j++){
                    trace+=dmat_symm(_j,_j)*threecenter(_j,_j);
                    for(unsigned _k=0;_k<_j;_k++){
                    trace+=2*dmat_symm(_j,_k)*threecenter(_j,_k);
                    }
                }
                Itilde(_i,0)=trace;
            }
            //cout << "Itilde " <<Itilde << endl;
            const ub::matrix<double>K=ub::prod(_inverse_Coulomb,Itilde);
            //cout << "K " << K << endl;
            
            unsigned nthreads = 1;
            #ifdef _OPENMP
               nthreads = omp_get_max_threads();
            #endif
               std::vector<ub::matrix<double> >ERIS_thread;
               
               for(unsigned i=0;i<nthreads;++i){
                   ub::matrix<double> thread=ub::zero_matrix<double>(_ERIs.size1());
                   ERIS_thread.push_back(thread);
               }
            
            #pragma omp parallel for
            for (unsigned thread=0;thread<nthreads;++thread){
                for ( unsigned _i = thread; _i < K.size1(); _i+=nthreads){

                ERIS_thread[thread]+=_threecenter.getDatamatrix(_i)*K(_i,0);    
                //cout << "I " << _threecenter.getDatamatrix(_i) << endl;
                //cout<< "ERIs " <<_ERIs<< endl;
                }
            }
            for (unsigned thread=0;thread<nthreads;++thread){
                _ERIs+=ERIS_thread[thread];
            }    
            
            CalculateEnergy(dmatasarray);
            return;
        }
        
        

        void ERIs::CalculateERIs_4c_small_molecule(const ub::matrix<double> &DMAT) {

          _ERIs = ub::zero_matrix<double>(DMAT.size1(), DMAT.size2());
          const ub::vector<double> dmatasarray = DMAT.data();
          const ub::vector<double>& _4c_vector = _fourcenter.get_4c_vector();

          int dftBasisSize = DMAT.size1();
          int vectorSize = (dftBasisSize*(dftBasisSize+1))/2;
          #pragma omp parallel for
          for (unsigned _i = 0; _i < DMAT.size1(); _i++) {
            unsigned sum_i = (_i*(_i+1))/2;
            for (unsigned _j = _i; _j < DMAT.size2(); _j++) {
              unsigned _index_ij = DMAT.size2() * _i - sum_i + _j;
              unsigned _index_ij_kl_a = vectorSize * _index_ij - (_index_ij*(_index_ij+1))/2;
              for (unsigned _k = 0; _k < DMAT.size1(); _k++) {
                unsigned sum_k = (_k*(_k+1))/2;
                for (unsigned _l = _k; _l < DMAT.size2(); _l++) {
                  unsigned _index_kl = DMAT.size2() * _k - sum_k + _l;

                  unsigned _index_ij_kl = _index_ij_kl_a + _index_kl;
                  if (_index_ij > _index_kl) _index_ij_kl = vectorSize * _index_kl - (_index_kl*(_index_kl+1))/2 + _index_ij;

                  if (_l == _k) {
                    _ERIs(_i, _j) += DMAT(_k, _l) * _4c_vector(_index_ij_kl);
                  } else {
                    _ERIs(_i, _j) += 2. * DMAT(_k, _l) * _4c_vector(_index_ij_kl);
                  }

                }
              }
              _ERIs(_j, _i) = _ERIs(_i, _j);
            }
          }

          CalculateEnergy(dmatasarray);
          return;
        }
        
        
        void ERIs::CalculateEXX_4c_small_molecule(const ub::matrix<double> &DMAT) {

          _EXXs = ub::zero_matrix<double>(DMAT.size1(), DMAT.size2());
          const ub::vector<double> dmatasarray = DMAT.data();
          const ub::vector<double>& _4c_vector = _fourcenter.get_4c_vector();

          int dftBasisSize = DMAT.size1();
          int vectorSize = (dftBasisSize*(dftBasisSize+1))/2;
          #pragma omp parallel for
          for (unsigned _i = 0; _i < DMAT.size1(); _i++) {
            unsigned sum_i = (_i*(_i+1))/2;
            for (unsigned _j = _i; _j < DMAT.size2(); _j++) {
              unsigned _index_ij = DMAT.size2() * _i - sum_i + _j;
              unsigned _index_ij_kl_a = vectorSize * _index_ij - (_index_ij*(_index_ij+1))/2;
              for (unsigned _k = 0; _k < DMAT.size1(); _k++) {
                unsigned sum_k = (_k*(_k+1))/2;
                for (unsigned _l = _k; _l < DMAT.size2(); _l++) {
                  unsigned _index_kl = DMAT.size2() * _k - sum_k + _l;

                  unsigned _index_ij_kl = _index_ij_kl_a + _index_kl;
                  if (_index_ij > _index_kl) _index_ij_kl = vectorSize * _index_kl - (_index_kl*(_index_kl+1))/2 + _index_ij;

                  if (_l == _k) {
                    _EXXs(_i, _l) += DMAT(_j, _k) * _4c_vector(_index_ij_kl);
                  } else {
                    _EXXs(_i, _l) += 2. * DMAT(_j, _k) * _4c_vector(_index_ij_kl);
                  }

                }
              }
              _EXXs(_j, _i) = _EXXs(_i, _j);
            }
          }

          CalculateEXXEnergy(dmatasarray);
          return;
        }
        
        
        void ERIs::CalculateERIs_4c_direct(const AOBasis& dftbasis, const ub::matrix<double> &DMAT) {

          cout << endl;
          cout << "ERIS.cc ERIs::CalculateERIs_4c_direct" << endl;

          // Initialize ERIs matrix
          _ERIs = ub::zero_matrix<double>(DMAT.size1(), DMAT.size2());
          // Number of shells
          int numShells = dftbasis.getNumofShells();

          #pragma omp parallel for
          for (int iShell_3 = 0; iShell_3 < numShells; iShell_3++) {
            const AOShell* shell_3 = dftbasis.getShell(iShell_3);
            for (int iShell_4 = iShell_3; iShell_4 < numShells; iShell_4++) {
              const AOShell* shell_4 = dftbasis.getShell(iShell_4);
              for (int iShell_1 = iShell_3; iShell_1 < numShells; iShell_1++) {
                const AOShell* shell_1 = dftbasis.getShell(iShell_1);
                for (int iShell_2 = iShell_1; iShell_2 < numShells; iShell_2++) {
                  const AOShell* shell_2 = dftbasis.getShell(iShell_2);

                  // Get the current 4c block
                  ub::matrix<double> subMatrix = ub::zero_matrix<double>(shell_1->getNumFunc() * shell_2->getNumFunc(), shell_3->getNumFunc() * shell_4->getNumFunc());
                  bool nonzero = _fourcenter.FillFourCenterRepBlock(subMatrix, shell_1, shell_2, shell_3, shell_4);
                  
                  // If there are only zeros, we don't need to put anything in the ERIs matrix
                  if (!nonzero)
                    continue;
                  
                  // Begin fill ERIs matrix
                  
                  FillERIsBlock(DMAT, subMatrix, shell_1, shell_2, shell_3, shell_4);

                  // Symmetry 1 <--> 2
                  if (iShell_1 != iShell_2)
                    FillERIsBlock(DMAT, subMatrix, shell_2, shell_1, shell_3, shell_4);

                  // Symmetry 3 <--> 4
                  if (iShell_3 != iShell_4)
                    FillERIsBlock(DMAT, subMatrix, shell_1, shell_2, shell_4, shell_3);

                  // Symmetry 1 <--> 2 and 3 <--> 4
                  if (iShell_1 != iShell_2 && iShell_3 != iShell_4)
                    FillERIsBlock(DMAT, subMatrix, shell_2, shell_1, shell_4, shell_3);

                  // Symmetry (1, 2) <--> (3, 4)
                  if (iShell_1 != iShell_3) {
                    
                    // TODO
                    // We need to use the transposed version of "submatrix"!
                    // Do something smarter than copying the submatrix into a new transposed matrix.
                    ub::matrix<double> subMatrix2 = ub::trans(subMatrix);
                    
                    FillERIsBlock(DMAT, subMatrix2, shell_3, shell_4, shell_1, shell_2);

                    // Symmetry 1 <--> 2
                    if (iShell_1 != iShell_2)
                      FillERIsBlock(DMAT, subMatrix2, shell_3, shell_4, shell_2, shell_1);

                    // Symmetry 3 <--> 4
                    if (iShell_3 != iShell_4)
                      FillERIsBlock(DMAT, subMatrix2, shell_4, shell_3, shell_1, shell_2);

                    // Symmetry 1 <--> 2 and 3 <--> 4
                    if (iShell_1 != iShell_2 && iShell_3 != iShell_4)
                      FillERIsBlock(DMAT, subMatrix2, shell_4, shell_3, shell_2, shell_1);
                  }
                  
                  // End fill ERIs matrix
                } // End loop over shell 2
              } // End loop over shell 1
            } // End loop over shell 4
          } // End loop over shell 3
          
          // Fill lower triangular part using symmetry
          for (int i = 0; i < DMAT.size1(); i++)
            for (int j = i + 1; j < DMAT.size2(); j++)
              _ERIs(j, i) = _ERIs(i, j);

          CalculateEnergy(DMAT.data());
          return;
        }
        
        
        
        void ERIs::FillERIsBlock(const ub::matrix<double> &DMAT, const ub::matrix<double> &subMatrix, const AOShell* shell_1, const AOShell* shell_2, const AOShell* shell_3, const AOShell* shell_4) {

          for (int iFunc_3 = 0; iFunc_3 < shell_3->getNumFunc(); iFunc_3++) {
            int ind_3 = shell_3->getStartIndex() + iFunc_3;
            for (int iFunc_4 = 0; iFunc_4 < shell_4->getNumFunc(); iFunc_4++) {
              int ind_4 = shell_4->getStartIndex() + iFunc_4;

              // Symmetry
              if (ind_3 > ind_4)
                continue;

              // Column index in the current sub-matrix
              int ind_subm_34 = shell_3->getNumFunc() * iFunc_4 + iFunc_3;

              for (int iFunc_1 = 0; iFunc_1 < shell_1->getNumFunc(); iFunc_1++) {
                int ind_1 = shell_1->getStartIndex() + iFunc_1;
                for (int iFunc_2 = 0; iFunc_2 < shell_2->getNumFunc(); iFunc_2++) {
                  int ind_2 = shell_2->getStartIndex() + iFunc_2;

                  // Symmetry
                  if (ind_1 > ind_2)
                    continue;

                  // Row index in the current sub-matrix
                  int ind_subm_12 = shell_1->getNumFunc() * iFunc_2 + iFunc_1;

                  // Fill ERIs matrix
                  _ERIs(ind_3, ind_4) += (ind_1 == ind_2 ? 1.0 : 2.0) * DMAT(ind_1, ind_2) * subMatrix(ind_subm_12, ind_subm_34);
                } // End loop over functions in shell 2
              } // End loop over functions in shell 1
            } // End loop over functions in shell 4
          } // End loop over functions in shell 3
        }
        
        
        
        void ERIs::CalculateERIs_diagonals(const AOBasis& dftbasis) {
          
          cout << endl;
          cout << "ERIS.cc ERIs::CalculateERIs_diagonals" << endl;
          
          // Number of shells
          int numShells = dftbasis.getNumofShells();
          // Total number of functions
          int dftBasisSize = dftbasis.AOBasisSize();
          
          // TODO: Store in ERIs, use for pre-screening in direct computation
          ub::matrix<double> matrix = ub::zero_matrix<double>(dftBasisSize); // <ii|jj>
          
          for (int iShell_1 = 0; iShell_1 < numShells; iShell_1++) {
            const AOShell* shell_1 = dftbasis.getShell(iShell_1);
            for (int iShell_2 = iShell_1; iShell_2 < numShells; iShell_2++) {
              const AOShell* shell_2 = dftbasis.getShell(iShell_2);
              
              // Get the current 4c block
              ub::matrix<double> subMatrix = ub::zero_matrix<double>(shell_1->getNumFunc() * shell_1->getNumFunc(), shell_2->getNumFunc() * shell_2->getNumFunc());
              bool nonzero = _fourcenter.FillFourCenterRepBlock(subMatrix, shell_1, shell_1, shell_2, shell_2);
              
              if (!nonzero)
                continue;
              
              for (int iFunc_1 = 0; iFunc_1 < shell_1->getNumFunc(); iFunc_1++) {
                int ind_1 = shell_1->getStartIndex() + iFunc_1;
                for (int iFunc_2 = 0; iFunc_2 < shell_2->getNumFunc(); iFunc_2++) {
                  int ind_2 = shell_2->getStartIndex() + iFunc_2;

                  // Symmetry
                  if (ind_1 > ind_2)
                    continue;

                  // Row index in the current sub-matrix
                  int ind_subm_11 = shell_1->getNumFunc() * iFunc_1 + iFunc_1;
                  // Column index in the current sub-matrix
                  int ind_subm_22 = shell_2->getNumFunc() * iFunc_2 + iFunc_2;
                  
                  // Begin fill diagonal matrix
                  
                  matrix(ind_1, ind_2) = subMatrix(ind_subm_11, ind_subm_22);
                  
                  // Symmetry
                  if (ind_1 != ind_2)
                    matrix(ind_2, ind_1) = matrix(ind_1, ind_2);
                  
                  // End fill diagonal matrix
                } // End loop over functions in shell 2
              } // End loop over functions in shell 1
            } // End loop over shell 2
          } // End loop over shell 1
        }
		
		
        
         void ERIs::CalculateEnergy(const ub::vector<double> &dmatasarray){
            
            const ub::vector<double>& ERIsasarray=_ERIs.data();
            double energy=0.0;
           #pragma omp parallel for reduction(+:energy)
            for (unsigned _i=0;_i<ERIsasarray.size();_i++){
                energy+=dmatasarray[_i]*ERIsasarray[_i];
                
            }
            _ERIsenergy=energy;
            return;
        }
         
         
         void ERIs::CalculateEXXEnergy(const ub::vector<double> &dmatasarray){
            
            const ub::vector<double>& EXXsasarray=_EXXs.data();
            double energy=0.0;
           #pragma omp parallel for reduction(+:energy)
            for (unsigned _i=0;_i<EXXsasarray.size();_i++){
                energy+=dmatasarray[_i]*EXXsasarray[_i];
                
            }
            _EXXenergy=energy;
            return;
        }
        
        
        void ERIs::printERIs(){
          for (unsigned i=0; i< _ERIs.size1(); i++){
                for (unsigned j=0; j< _ERIs.size2();j++){
                    cout << "ERIs [" << i<<":"<<j<<"]="<<_ERIs(i,j)<<endl;
                }
            }
          return;
        }
        
        
        
        
        
    }}
