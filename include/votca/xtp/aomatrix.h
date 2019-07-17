/*
 *            Copyright 2009-2019 The VOTCA Development Team
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

#pragma once
#ifndef VOTCA_XTP_AOMATRIX_H
#define VOTCA_XTP_AOMATRIX_H

#include <votca/xtp/aobasis.h>
#include <votca/xtp/multiarray.h>
#include <votca/xtp/staticsite.h>

namespace votca {
namespace xtp {

// base class for 1D atomic orbital matrix types (overlap, Coulomb, ESP)
template <class T>
class AOMatrix {
 public:
  int Dimension() { return _aomatrix.rows(); }
  const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>& Matrix() const {
    return _aomatrix;
  }
  void Fill(const AOBasis& aobasis);

 protected:
  virtual void FillBlock(
      Eigen::Block<Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>>& matrix,
      const AOShell& shell_row, const AOShell& shell_col) const = 0;
  Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> _aomatrix;
};

// derived class for atomic orbital nuclear potential
class AOESP : public AOMatrix<double> {
 public:
  void Fillnucpotential(const AOBasis& aobasis, const QMMolecule& atoms);
  void Fillextpotential(
      const AOBasis& aobasis,
      const std::vector<std::unique_ptr<StaticSite>>& externalsites);
  const Eigen::MatrixXd& getNuclearpotential() const {
    return _nuclearpotential;
  }
  const Eigen::MatrixXd& getExternalpotential() const {
    return _externalpotential;
  }
  void setPosition(const Eigen::Vector3d& r) { _r = r; };

 protected:
  void FillBlock(Eigen::Block<Eigen::MatrixXd>& matrix,
                 const AOShell& shell_row, const AOShell& shell_col) const;

 private:
  Eigen::Vector3d _r;
  Eigen::MatrixXd _nuclearpotential;
  Eigen::MatrixXd _externalpotential;
};

// derived class for Effective Core Potentials
class AOECP : public AOMatrix<double> {
 public:
  void setECP(const AOBasis* ecp) { _ecp = ecp; }

 protected:
  void FillBlock(Eigen::Block<Eigen::MatrixXd>& matrix,
                 const AOShell& shell_row, const AOShell& shell_col) const;

 private:
  const AOBasis* _ecp;
  Eigen::MatrixXd calcVNLmatrix(
      int lmax_ecp, const Eigen::Vector3d& posC,
      const AOGaussianPrimitive& g_row, const AOGaussianPrimitive& g_col,
      const Eigen::Matrix<int, 4, 5>& power_ecp,
      const Eigen::Matrix<double, 4, 5>& gamma_ecp,
      const Eigen::Matrix<double, 4, 5>& pref_ecp) const;

  void getBLMCOF(int lmax_ecp, int lmax_dft, const Eigen::Vector3d& pos,
                 tensor3d& BLC, tensor3d& C) const;
  Eigen::VectorXd CalcNorms(double decay, int size) const;
  Eigen::VectorXd CalcInt_r_exp(int nmax, double decay) const;
};

// derived class for kinetic energy
class AOKinetic : public AOMatrix<double> {
 protected:
  void FillBlock(Eigen::Block<Eigen::MatrixXd>& matrix,
                 const AOShell& shell_row, const AOShell& shell_col) const;
};

// derived class for atomic orbital overlap
class AOOverlap : public AOMatrix<double> {
 public:
  Eigen::MatrixXd FillShell(const AOShell& shell) const;
  int Removedfunctions() const { return removedfunctions; }
  double SmallestEigenValue() const { return smallestEigenvalue; }

  Eigen::MatrixXd Pseudo_InvSqrt(double etol);
  Eigen::MatrixXd Sqrt();

 protected:
  void FillBlock(Eigen::Block<Eigen::MatrixXd>& matrix,
                 const AOShell& shell_row, const AOShell& shell_col) const;

 private:
  int removedfunctions;
  double smallestEigenvalue;
};

class AODipole_Potential : public AOMatrix<double> {
 public:
  void Fillextpotential(
      const AOBasis& aobasis,
      const std::vector<std::unique_ptr<StaticSite>>& externalsites);
  const Eigen::MatrixXd& getExternalpotential() const {
    return _externalpotential;
  }

 protected:
  void FillBlock(Eigen::Block<Eigen::MatrixXd>& matrix,
                 const AOShell& shell_row, const AOShell& shell_col) const;

 private:
  void setSite(const StaticSite* site) { _site = site; };
  const StaticSite* _site;
  Eigen::MatrixXd _externalpotential;
};

class AOQuadrupole_Potential : public AOMatrix<double> {
 public:
  void Fillextpotential(
      const AOBasis& aobasis,
      const std::vector<std::unique_ptr<StaticSite>>& externalsites);
  const Eigen::MatrixXd& getExternalpotential() const {
    return _externalpotential;
  }

 protected:
  void FillBlock(Eigen::Block<Eigen::MatrixXd>& matrix,
                 const AOShell& shell_row, const AOShell& shell_col) const;

 private:
  void setSite(const StaticSite* site) { _site = site; };

  const StaticSite* _site;
  Eigen::MatrixXd _externalpotential;
};

// derived class for atomic orbital Coulomb interaction
class AOCoulomb : public AOMatrix<double> {
 public:
  Eigen::MatrixXd Pseudo_InvSqrt_GWBSE(const AOOverlap& auxoverlap,
                                       double etol);
  Eigen::MatrixXd Pseudo_InvSqrt(double etol);
  int Removedfunctions() const { return removedfunctions; }

 protected:
  void FillBlock(Eigen::Block<Eigen::MatrixXd>& matrix,
                 const AOShell& shell_row, const AOShell& shell_col) const;

 private:
  int removedfunctions;
};

class AOPlanewave : public AOMatrix<std::complex<double>> {
 public:
  void Fillextpotential(const AOBasis& aobasis,
                        const std::vector<Eigen::Vector3d>& kpoints);
  Eigen::MatrixXd getExternalpotential() const {
    return _externalpotential.real();
  }

 protected:
  void FillBlock(Eigen::Block<Eigen::MatrixXcd>& matrix,
                 const AOShell& shell_row, const AOShell& shell_col) const;

 private:
  void setkVector(const Eigen::Vector3d& k) { _k = k; };
  Eigen::Vector3d _k;
  Eigen::MatrixXcd _externalpotential;
};

}  // namespace xtp
}  // namespace votca

#endif  // VOTCA_XTP_AOMATRIX_H
