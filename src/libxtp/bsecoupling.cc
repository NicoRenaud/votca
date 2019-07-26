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

#include <boost/format.hpp>
#include <votca/tools/constants.h>
#include <votca/xtp/aomatrix.h>
#include <votca/xtp/bsecoupling.h>

#include "votca/xtp/bse.h"
#include "votca/xtp/bse_operator.h"

namespace votca {
namespace xtp {
using namespace std;
using boost::format;
using namespace tools;

void BSECoupling::Initialize(Property& options) {

  std::string key = Identify();
  _doSinglets = false;
  _doTriplets = false;
  _output_perturbation = false;

  string spintype = options.get(key + ".spin").as<string>();
  if (spintype == "all") {
    _doSinglets = true;
    _doTriplets = true;
  } else if (spintype == "triplet") {
    _doTriplets = true;
  } else if (spintype == "singlet") {
    _doSinglets = true;
  } else {
    throw std::runtime_error(
        (boost::format(
             "Choice % for type not known. Available singlet,triplet,all") %
         spintype)
            .str());
  }

  if (options.exists(key + ".algorithm")) {
    string algorithm = options.get(key + ".algorithm").as<string>();
    if (algorithm == "perturbation") {
      _output_perturbation = true;
    }
  }

  _levA = options.get(key + ".moleculeA.states").as<int>();
  _levB = options.get(key + ".moleculeB.states").as<int>();
  _occA = options.get(key + ".moleculeA.occLevels").as<int>();
  _occB = options.get(key + ".moleculeB.occLevels").as<int>();
  _unoccA = options.get(key + ".moleculeA.unoccLevels").as<int>();
  _unoccB = options.get(key + ".moleculeB.unoccLevels").as<int>();
}

void BSECoupling::WriteToProperty(const Orbitals& orbitalsA,
                                  const Orbitals& orbitalsB, Property& summary,
                                  const QMState& stateA,
                                  const QMState& stateB) {
  Property& coupling_summary = summary.add("coupling", "");
  double energyA = 0;
  double energyB = 0;
  double JAB_pert = 0;
  double JAB_diag = 0;
  if (stateA.Type() == QMStateType::Singlet) {
    energyA =
        orbitalsA.BSESinglets().eigenvalues()(stateA.Index()) * conv::hrt2ev;
    energyB =
        orbitalsB.BSESinglets().eigenvalues()(stateB.Index()) * conv::hrt2ev;
    JAB_pert = getSingletCouplingElement(stateA.Index(), stateB.Index(), 0);
    JAB_diag = getSingletCouplingElement(stateA.Index(), stateB.Index(), 1);
  } else if (stateA.Type() == QMStateType::Triplet) {
    energyA =
        orbitalsA.BSETriplets().eigenvalues()(stateA.Index()) * conv::hrt2ev;
    energyB =
        orbitalsB.BSETriplets().eigenvalues()(stateB.Index()) * conv::hrt2ev;
    JAB_pert = getTripletCouplingElement(stateA.Index(), stateB.Index(), 0);
    JAB_diag = getTripletCouplingElement(stateA.Index(), stateB.Index(), 1);
  }
  coupling_summary.setAttribute("stateA", stateA.ToString());
  coupling_summary.setAttribute("stateB", stateB.ToString());
  coupling_summary.setAttribute("eA", (format("%1$1.6e") % energyA).str());
  coupling_summary.setAttribute("eB", (format("%1$1.6e") % energyB).str());
  coupling_summary.setAttribute("j_pert", (format("%1$1.6e") % JAB_pert).str());
  coupling_summary.setAttribute("j_diag", (format("%1$1.6e") % JAB_diag).str());
}

void BSECoupling::Addoutput(Property& type_summary, const Orbitals& orbitalsA,
                            const Orbitals& orbitalsB) {
  tools::Property& bsecoupling = type_summary.add(Identify(), "");
  string algorithm = "j_diag";
  if (_output_perturbation) {
    algorithm = "j_pert";
  }
  if (_doSinglets) {
    QMStateType singlet = QMStateType(QMStateType::Singlet);
    Property& singlet_summary = bsecoupling.add(singlet.ToLongString(), "");
    singlet_summary.setAttribute("algorithm", algorithm);
    for (int stateA = 0; stateA < _levA; ++stateA) {
      QMState qmstateA = QMState(singlet, stateA, false);
      for (int stateB = 0; stateB < _levB; ++stateB) {
        QMState qmstateB = QMState(singlet, stateB, false);
        WriteToProperty(orbitalsA, orbitalsB, singlet_summary, qmstateA,
                        qmstateB);
      }
    }
  }

  if (_doTriplets) {
    QMStateType triplet = QMStateType(QMStateType::Triplet);
    Property& triplet_summary = bsecoupling.add(triplet.ToLongString(), "");
    triplet_summary.setAttribute("algorithm", algorithm);
    for (int stateA = 0; stateA < _levA; ++stateA) {
      QMState qmstateA = QMState(triplet, stateA, false);
      for (int stateB = 0; stateB < _levB; ++stateB) {
        QMState qmstateB = QMState(triplet, stateB, false);
        WriteToProperty(orbitalsA, orbitalsB, triplet_summary, qmstateA,
                        qmstateB);
      }
    }
  }
}

double BSECoupling::getSingletCouplingElement(int levelA, int levelB,
                                              int methodindex) {
  return JAB_singlet[methodindex](levelA, levelB + _levA) *
         votca::tools::conv::hrt2ev;
}

double BSECoupling::getTripletCouplingElement(int levelA, int levelB,
                                              int methodindex) {
  return JAB_triplet[methodindex](levelA, levelB + _levA) *
         votca::tools::conv::hrt2ev;
}

Eigen::MatrixXd BSECoupling::SetupCTStates(int bseA_vtotal, int bseB_vtotal,
                                           int bseAB_vtotal, int bseAB_ctotal,
                                           const Eigen::MatrixXd& A_AB,
                                           const Eigen::MatrixXd& B_AB) const {

  int noAB = _occA * _unoccB;
  int noBA = _unoccA * _occB;
  int bseAB_total = bseAB_vtotal + bseAB_ctotal;
  int bseAB_size = bseAB_vtotal * bseAB_ctotal;
  Eigen::MatrixXd CTstates = Eigen::MatrixXd::Zero(bseAB_size, noAB + noBA);

  auto A_occ = A_AB.block(0, bseA_vtotal - _occA, bseAB_total, _occA);
  auto A_unocc = A_AB.block(0, bseA_vtotal, bseAB_total, _unoccA);
  auto B_occ = B_AB.block(0, bseB_vtotal - _occB, bseAB_total, _occB);
  auto B_unocc = B_AB.block(0, bseB_vtotal, bseAB_total, _unoccB);

  Eigen::MatrixXd A_occ_occ = A_occ.topRows(bseAB_vtotal);
  Eigen::MatrixXd A_occ_unocc = A_occ.bottomRows(bseAB_ctotal);
  Eigen::MatrixXd B_unocc_occ = B_unocc.topRows(bseAB_vtotal);
  Eigen::MatrixXd B_unocc_unocc = B_unocc.bottomRows(bseAB_ctotal);
  // notation AB is CT states with A+B-, BA is the counterpart
  // Setting up CT-states:
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   Setting up CT-states" << flush;

  // Number of A+B- states

  for (int a_occ = 0; a_occ < _occA; a_occ++) {
    for (int b_unocc = 0; b_unocc < _unoccB; b_unocc++) {
      int index = a_occ * _unoccB + b_unocc;
      Eigen::MatrixXd Coeff =
          B_unocc_unocc.col(b_unocc) * A_occ_occ.col(a_occ).transpose();
      Coeff += A_occ_unocc.col(a_occ) * B_unocc_occ.col(b_unocc).transpose();
      CTstates.col(index) =
          Eigen::Map<Eigen::VectorXd>(Coeff.data(), bseAB_size);
    }
  }
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "  " << noBA << " CT states A+B- created" << flush;
  Eigen::MatrixXd A_unocc_occ = A_unocc.topRows(bseAB_vtotal);
  Eigen::MatrixXd A_unocc_unocc = A_unocc.bottomRows(bseAB_ctotal);
  Eigen::MatrixXd B_occ_occ = B_occ.topRows(bseAB_vtotal);
  Eigen::MatrixXd B_occ_unocc = B_occ.bottomRows(bseAB_ctotal);

  for (int b_occ = 0; b_occ < _occB; b_occ++) {
    for (int a_unocc = 0; a_unocc < _unoccA; a_unocc++) {
      int index = b_occ * _unoccA + a_unocc + noAB;
      Eigen::MatrixXd Coeff =
          A_unocc_unocc.col(a_unocc) * B_occ_occ.col(b_occ).transpose();
      Coeff += B_occ_unocc.col(b_occ) * A_unocc_occ.col(a_unocc).transpose();
      CTstates.col(index) =
          Eigen::Map<Eigen::VectorXd>(Coeff.data(), bseAB_size);
    }
  }
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "  " << noBA << " CT states A-B+ created" << flush;
  return CTstates;
}

/**
 * \brief evaluates electronic couplings
 *
 * @param _orbitalsA molecular orbitals of molecule A
 * @param _orbitalsB molecular orbitals of molecule B
 * @param _orbitalsAB molecular orbitals of the dimer AB
 */
void BSECoupling::CalculateCouplings(const Orbitals& orbitalsA,
                                     const Orbitals& orbitalsB,
                                     const Orbitals& orbitalsAB) {
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "  Calculating exciton couplings" << flush;
  // set the parallelization
  XTP_LOG(logDEBUG, *_pLog) << TimeStamp() << " Using "
                            << OPENMP::getMaxThreads() << " threads" << flush;

  CheckAtomCoordinates(orbitalsA, orbitalsB, orbitalsAB);

  // constructing the direct product orbA x orbB
  int basisB = orbitalsB.getBasisSetSize();
  int basisA = orbitalsA.getBasisSetSize();
  if ((basisA == 0) || (basisB == 0)) {
    throw std::runtime_error("Basis set size is not stored in monomers");
  }

  // get exciton information of molecule A
  int bseA_cmax = orbitalsA.getBSEcmax();
  int bseA_cmin = orbitalsA.getBSEcmin();
  int bseA_vmax = orbitalsA.getBSEvmax();
  int bseA_vmin = orbitalsA.getBSEvmin();
  int bseA_vtotal = bseA_vmax - bseA_vmin + 1;
  int bseA_ctotal = bseA_cmax - bseA_cmin + 1;
  int bseA_total = bseA_vtotal + bseA_vtotal;
  int bseA_size = bseA_vtotal * bseA_ctotal;
  int bseA_singlet_exc = orbitalsA.BSESinglets().eigenvectors().cols();
  int bseA_triplet_exc = orbitalsA.BSETriplets().eigenvectors().cols();

  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   molecule A has " << bseA_singlet_exc
      << " singlet excitons with dimension " << bseA_size << flush;
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   molecule A has " << bseA_triplet_exc
      << " triplet excitons with dimension " << bseA_size << flush;

  // get exciton information of molecule B
  int bseB_cmax = orbitalsB.getBSEcmax();
  int bseB_cmin = orbitalsB.getBSEcmin();
  int bseB_vmax = orbitalsB.getBSEvmax();
  int bseB_vmin = orbitalsB.getBSEvmin();
  int bseB_vtotal = bseB_vmax - bseB_vmin + 1;
  int bseB_ctotal = bseB_cmax - bseB_cmin + 1;
  int bseB_total = bseB_vtotal + bseB_ctotal;
  int bseB_size = bseB_vtotal * bseB_ctotal;
  int bseB_singlet_exc = orbitalsB.BSESinglets().eigenvectors().cols();
  int bseB_triplet_exc = orbitalsB.BSETriplets().eigenvectors().cols();

  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   molecule B has " << bseB_singlet_exc
      << " singlet excitons with dimension " << bseB_size << flush;
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   molecule B has " << bseB_triplet_exc
      << " triplet excitons with dimension " << bseB_size << flush;

  if (_levA > bseA_singlet_exc) {
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp()
        << "  Number of excitons you want is greater than stored for molecule "
           "A. Setting to max number available"
        << flush;
    _levA = bseA_singlet_exc;
  }
  if (_levB > bseB_singlet_exc) {
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp()
        << "  Number of excitons you want is greater than stored for molecule "
           "B. Setting to max number available"
        << flush;
    _levB = bseB_singlet_exc;
  }

  if (_levA > bseA_singlet_exc) {
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp()
        << "  Number of Frenkel states you want is greater than stored for "
           "molecule A. Setting to max number available"
        << flush;
    _levA = bseA_singlet_exc;
  }
  if (_levB > bseB_singlet_exc) {
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp()
        << "  Number of Frenkel states you want is greater than stored for "
           "molecule B. Setting to max number available"
        << flush;
    _levB = bseB_singlet_exc;
  }

  if (_unoccA > bseA_ctotal) {
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp()
        << "  Number of occupied orbitals in molecule A for CT creation "
           "exceeds number of KS-orbitals in BSE"
        << flush;
    _unoccA = bseA_ctotal;
  } else if (_unoccA < 0) {
    _unoccA = bseA_ctotal;
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp()
        << "  Number of occupied orbitals in molecule B for CT creation "
           "exceeds number of KS-orbitals in BSE"
        << flush;
  }
  if (_unoccB > bseB_ctotal) {
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp()
        << "  Number of occupied orbitals in molecule B for CT creation "
           "exceeds number of KS-orbitals in BSE"
        << flush;
    _unoccB = bseB_ctotal;
  } else if (_unoccB < 0) {
    _unoccB = bseB_ctotal;
  }

  if (_occA > bseA_vtotal) {
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp()
        << "  Number of unoccupied orbitals in molecule A for CT creation "
           "exceeds number of KS-orbitals in BSE"
        << flush;
    _occA = bseA_vtotal;
  } else if (_occA < 0) {
    _occA = bseA_vtotal;
  }
  if (_occB > bseB_vtotal) {
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp()
        << "  Number of unoccupied orbitals in molecule B for CT creation "
           "exceeds number of KS-orbitals in BSE"
        << flush;
    _occB = bseB_vtotal;
  } else if (_occB < 0) {
    _occB = bseB_vtotal;
  }

  // get exciton information of pair AB
  int bseAB_cmax = orbitalsAB.getBSEcmax();
  int bseAB_cmin = orbitalsAB.getBSEcmin();
  int bseAB_vmax = orbitalsAB.getBSEvmax();
  int bseAB_vmin = orbitalsAB.getBSEvmin();
  int basisAB = orbitalsAB.getBasisSetSize();
  int bseAB_vtotal = bseAB_vmax - bseAB_vmin + 1;
  int bseAB_ctotal = bseAB_cmax - bseAB_cmin + 1;
  int bseAB_total = bseAB_vtotal + bseAB_ctotal;
  int bseAB_size = bseAB_vtotal * bseAB_ctotal;

  // DFT levels of monomers can be reduced to those used in BSE
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   levels used in BSE of molA: " << bseA_vmin << " to "
      << bseA_cmax << " total: " << bseA_total << flush;
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   levels used in BSE of molB: " << bseB_vmin << " to "
      << bseB_cmax << " total: " << bseB_total << flush;
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   levels used in BSE of dimer AB: " << bseAB_vmin
      << " to " << bseAB_cmax << " total: " << bseAB_total << flush;

  auto MOsA =
      orbitalsA.MOs().eigenvectors().block(0, bseA_vmin, basisA, bseA_total);
  auto MOsB =
      orbitalsB.MOs().eigenvectors().block(0, bseB_vmin, basisB, bseB_total);
  auto MOsAB =
      orbitalsB.MOs().eigenvectors().block(0, bseAB_vmin, basisAB, bseAB_total);

  XTP_LOG(logDEBUG, *_pLog) << "Calculating overlap matrix for basisset: "
                            << orbitalsAB.getDFTbasisName() << flush;
  Eigen::MatrixXd overlap = CalculateOverlapMatrix(orbitalsAB) * MOsAB;

  XTP_LOG(logDEBUG, *_pLog)
      << "Projecting monomers onto dimer orbitals" << flush;
  Eigen::MatrixXd A_AB = overlap.topRows(basisA).transpose() * MOsA;
  Eigen::MatrixXd B_AB = overlap.bottomRows(basisB).transpose() * MOsB;
  Eigen::VectorXd mag_A = A_AB.colwise().squaredNorm();
  if (mag_A.any() < 0.95) {
    XTP_LOG(logERROR, *_pLog)
        << "\nWarning: "
        << "Projection of orbitals of monomer A on dimer is insufficient,mag="
        << mag_A.minCoeff() << flush;
  }
  Eigen::VectorXd mag_B = B_AB.colwise().squaredNorm();
  if (mag_B.any() < 0.95) {
    XTP_LOG(logERROR, *_pLog)
        << "\nWarning: "
        << "Projection of orbitals of monomer B on dimer is insufficient,mag="
        << mag_B.minCoeff() << flush;
  }

  Eigen::MatrixXd CTStates = SetupCTStates(
      bseA_vtotal, bseB_vtotal, bseAB_vtotal, bseAB_ctotal, A_AB, B_AB);

  Eigen::MatrixXd Kap = Eigen::MatrixXd::Zero(bseA_size, bseAB_size);

  // these 4 matrixes, matrix(i,j) contains the j-th dimer MO component of the
  // i-th excitation

  _kap.resize(bseA_size, bseAB_size);
#pragma omp parallel for
  for (int i_bseA = 0; i_bseA < bseA_size; i_bseA++) {
    for (int i_bseAB = 0; i_bseAB < bseAB_size; i_bseAB++) {
      _kap(i_bseA, i_bseAB) =
          psi_AxB_dimer_basis(combA(i_bseA, 0), combAB(i_bseAB, 0)) *
          psi_AxB_dimer_basis(combA(i_bseA, 1), combAB(i_bseAB, 1));
    }
  }

  _kbp.resize(bseB_size, bseAB_size);
#pragma omp parallel for
  for (int i_bseB = 0; i_bseB < bseB_size; i_bseB++) {
    for (int i_bseAB = 0; i_bseAB < bseAB_size; i_bseAB++) {
      _kbp(i_bseB, i_bseAB) =
          psi_AxB_dimer_basis(combB(i_bseB, 0), combAB(i_bseAB, 0)) *
          psi_AxB_dimer_basis(combB(i_bseB, 1), combAB(i_bseAB, 1));
    }
  }

  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   construct projection of product functions "
      << flush;

  AOBasis dftbasis = orbitalsAB.SetupDftBasis() AOBasis auxbasis =
      orbitalsAB.SetupAuxBasis();
  TCMatrix_gwbse Mmn;
  // rpamin here, because RPA needs till rpamin
  Mmn.Initialize(auxbasis.AOBasisSize(), orbitalsAB.getRPAmin(),
                 orbitalsAB.getGWAmax(), orbitalsAB.getRPAmin(),
                 orbitalsAB.getRPAmax());
  Mmn.Fill(auxbasis, dftbasis, orbitalsAB.MOs().eigenvectors());

  const Eigen::MatrixXd& qpcoeff = orbitalsAB.QPdiag().eigenvectors();
  Eigen::MatrixXd Hqp = qpcoeff *
                        orbitalsAB.QPdiag().eigenvalues().asDiagonal() *
                        qpcoeff.transpose();
  BSE::options opt;
  opt.cmax = orbitalsAB.getBSEcmax();
  opt.homo = orbitalsAB.getHomo();
  opt.qpmin = orbitalsAB.getGWAmin();
  opt.rpamax = orbitalsAB.getRPAmax();
  opt.rpamin = orbitalsAB.getRPAmin();
  opt.useTDA = true;
  opt.vmin = orbitalsAB.getBSEvmin();
  BSE bse(orbitalsAB, *_pLog, Mmn, Hqp);
  bse.configure(opt);

  // now the different spin types
  if (_doSinglets) {
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp() << "   Evaluating singlets" << flush;
    XTP_LOG(logDEBUG, *_pLog) << TimeStamp() << "   Setup Hamiltonian" << flush;
    const Eigen::MatrixXd bseA =
        orbitalsA.BSESinglets().eigenvectors().leftCols(_levA);
    const Eigen::MatrixXd bseB =
        orbitalsB.BSESinglets().eigenvectors().leftCols(_levB);

    JAB_singlet = ProjectExcitons(bseA, bseB, bse.getSingletOperator_TDA());
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp() << "   calculated singlet couplings " << flush;
  }

  if (_doTriplets) {
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp() << "   Evaluating triplets" << flush;

    const Eigen::MatrixXd bseA =
        orbitalsA.BSETriplets().eigenvectors().leftCols(_levA);
    const Eigen::MatrixXd bseB =
        orbitalsB.BSETriplets().eigenvectors().leftCols(_levB);
    JAB_triplet = ProjectExcitons(bseA, bseB, bse.getTripletOperator_TDA());
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp() << "   calculated triplet couplings " << flush;
  }

  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "  Done with exciton couplings" << flush;
  return;
};

template <class BSE_OPERATOR>
std::array<Eigen::MatrixXd, 2> BSECoupling::ProjectExcitons(
    const Eigen::MatrixXd& bseA, const Eigen::MatrixXd& bseB, BSE_OPERATOR H) {

  // get projection of monomer excitons on dimer product functions
  Eigen::MatrixXd proj_excA = bseA.transpose() * _kap;
  Eigen::MatrixXd proj_excB = bseB.transpose() * _kbp;

  int bse_exc = _levA + _levB;
  int ctABsize = ctAB.rows();
  int ctBAsize = ctBA.rows();
  int ct = ctAB.rows() + ctBA.rows();
  int nobasisfunc = H.rows();

  Eigen::MatrixXd fe_states = Eigen::MatrixXd::Zero(bse_exc, nobasisfunc);
  fe_states.topRows(_levA) = proj_excA;
  fe_states.bottomRows(_levB) = proj_excB;

  Eigen::MatrixXd ct_states = Eigen::MatrixXd::Zero(ct, nobasisfunc);
  if (ct > 0) {
    // orthogonalize ct-states with respect to the FE states.
    XTP_LOG(logDEBUG, *_pLog)
        << TimeStamp() << " Orthogonalizing CT-states with respect to FE-states"
        << flush;

    if (ctABsize > 0) {
      ct_states.topRows(ctABsize) = ctAB;
    }
    if (ctBAsize > 0) {
      ct_states.bottomRows(ctBAsize) = ctBA;
    }

    // orthogonalize ct-states with respect to FE states
    {
      Eigen::MatrixXd correction =
          ct_states * fe_states.transpose() * fe_states;
      ct_states -= correction;
    }
    // normalize
    Eigen::VectorXd norm = ct_states.rowwise().norm();
    for (int i = 0; i < ct; i++) {
      ct_states.row(i) /= norm(i);
    }
    int minstateindex = 0;
    double minnorm = norm.minCoeff(&minstateindex);
    if (minnorm < 0.95) {
      XTP_LOG(logDEBUG, *_pLog)
          << TimeStamp() << " WARNING: CT-state " << minstateindex
          << " norm is only " << minnorm << flush;
    }
  }
  Eigen::MatrixXd projection(nobasisfunc, bse_exc + ct);
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << " merging projections into one vector  " << flush;
  projection.leftCols(bse_exc) = fe_states;

  if (ct > 0) {
    projection.rightCols(ct) = ct_states;
  }
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   Setting up coupling matrix size " << bse_exc + ct
      << "x" << bse_exc + ct << flush;
  // matrix _J
  //  E_A         J_AB        J_A_ABCT        J_A_BACT
  //  J_BA        E_B         J_B_ABCT        J_B_BACT
  //  J_ABCT_A    J_ABCT_B    E_ABCT          J_ABCT_BACT
  //  J_BACT_A   J_BACT_B    J_BACT_ABCT     E_BACT

  // this only works for hermitian/symmetric H so only in TDA

  Eigen::MatrixXd temp = H * projection;
  Eigen::MatrixXd J_dimer = projection.transpose() * temp;

  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   Setting up overlap matrix size " << bse_exc + ct
      << "x" << bse_exc + ct << flush;
  Eigen::MatrixXd S_dimer = projection.transpose() * projection;

  projection.resize(0, 0);
  if (tools::globals::verbose && bse_exc + ct < 100) {
    XTP_LOG(logDEBUG, *_pLog)
        << "---------------------------------------" << flush;
    XTP_LOG(logDEBUG, *_pLog) << "_J_dimer[Ryd]" << flush;

    XTP_LOG(logDEBUG, *_pLog) << J_dimer << flush;
    XTP_LOG(logDEBUG, *_pLog) << "_S_dimer" << flush;

    XTP_LOG(logDEBUG, *_pLog) << S_dimer << flush;
    XTP_LOG(logDEBUG, *_pLog)
        << "---------------------------------------" << flush;
  }

  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> es(S_dimer);
  Eigen::MatrixXd Sm1 = es.operatorInverseSqrt();
  J_dimer = Sm1 * J_dimer * Sm1;

  if (tools::globals::verbose && bse_exc + ct < 100) {
    XTP_LOG(logDEBUG, *_pLog)
        << "---------------------------------------" << flush;
    XTP_LOG(logDEBUG, *_pLog) << "_J_ortho[Ryd]" << flush;
    XTP_LOG(logDEBUG, *_pLog) << J_dimer << flush;
    XTP_LOG(logDEBUG, *_pLog) << "_S-1/2" << flush;
    XTP_LOG(logDEBUG, *_pLog) << Sm1 << flush;
    XTP_LOG(logDEBUG, *_pLog)
        << "---------------------------------------" << flush;
  }
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   Smallest value of dimer overlapmatrix is "
      << es.eigenvalues()(0) << flush;

  std::array<Eigen::MatrixXd, 2> J;

  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "   Running Perturbation algorithm" << flush;
  J[0] = Perturbation(J_dimer);
  XTP_LOG(logDEBUG, *_pLog)
      << TimeStamp() << "    Running Projection algorithm" << flush;
  J[1] = Fulldiag(J_dimer);

  if (tools::globals::verbose) {
    XTP_LOG(logDEBUG, *_pLog)
        << "---------------------------------------" << flush;
    XTP_LOG(logDEBUG, *_pLog) << "Jeff_pert[Hrt]" << flush;
    XTP_LOG(logDEBUG, *_pLog) << J[0] << flush;
    XTP_LOG(logDEBUG, *_pLog) << "Jeff_diag[Hrt]" << flush;
    XTP_LOG(logDEBUG, *_pLog) << J[1] << flush;
    XTP_LOG(logDEBUG, *_pLog)
        << "---------------------------------------" << flush;
  }

  return J;
}

Eigen::MatrixXd BSECoupling::Perturbation(const Eigen::MatrixXd& J_dimer) {
  int bse_exc = _levA + _levB;
  int ct = ctAB.rows() + ctBA.rows();
  Eigen::MatrixXd Jmatrix = Eigen::MatrixXd::Zero(bse_exc, bse_exc);
  bool diag_ct = true;
  Eigen::MatrixXd J_result = J_dimer;
  if (ct > 0 && diag_ct) {
    Eigen::MatrixXd transformation =
        Eigen::MatrixXd::Identity(bse_exc + ct, bse_exc + ct);
    Eigen::MatrixXd Ct = J_dimer.bottomRightCorner(ct, ct);
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> es(Ct);
    transformation.bottomRightCorner(ct, ct) = es.eigenvectors();
    Ct.resize(0, 0);

    if (tools::globals::verbose) {
      XTP_LOG(logDEBUG, *_pLog) << "FE state hamiltonian" << flush;
      XTP_LOG(logDEBUG, *_pLog)
          << J_dimer.block(0, 0, bse_exc, bse_exc) << flush;
      if (ct > 0) {
        XTP_LOG(logDEBUG, *_pLog) << "eigenvalues of CT states" << flush;
        XTP_LOG(logDEBUG, *_pLog) << es.eigenvalues() << flush;
      }
    }

    J_result = transformation.transpose() * J_dimer * transformation;
    if (tools::globals::verbose && bse_exc + ct < 100) {
      XTP_LOG(logDEBUG, *_pLog)
          << "---------------------------------------" << flush;
      XTP_LOG(logDEBUG, *_pLog) << "_J_ortho[Hrt] CT-state diag" << flush;
      XTP_LOG(logDEBUG, *_pLog) << J_result << flush;
      XTP_LOG(logDEBUG, *_pLog)
          << "---------------------------------------" << flush;
    }
  }
  for (int stateA = 0; stateA < _levA; stateA++) {
    double Ea = J_result(stateA, stateA);
    for (int stateB = 0; stateB < _levB; stateB++) {
      int stateBd = stateB + _levA;
      XTP_LOG(logDEBUG, *_pLog)
          << TimeStamp() << "   Calculating coupling between exciton A"
          << stateA + 1 << " and exciton B" << stateB + 1 << flush;
      double J = J_result(stateA, stateBd);

      double Eb = J_result(stateBd, stateBd);
      for (int k = bse_exc; k < (bse_exc + ct); k++) {
        double Eab = J_result(k, k);
        if (std::abs(Eab - Ea) < 0.001) {
          XTP_LOG(logDEBUG, *_pLog)
              << TimeStamp() << "Energydifference between state A "
              << stateA + 1 << "and CT state " << k + 1 << " is " << Eab - Ea
              << "[Hrt]" << flush;
        }
        if (std::abs(Eab - Eb) < 0.001) {
          XTP_LOG(logDEBUG, *_pLog)
              << TimeStamp() << "Energydifference between state B "
              << stateB + 1 << "and CT state " << k + 1 << " is " << Eab - Eb
              << "[Hrt]" << flush;
        }
        J += 0.5 * J_result(k, stateA) * J_result(k, stateBd) *
             (1 / (Ea - Eab) + 1 / (Eb - Eab));  // Have no clue why 0.5
      }
      Jmatrix(stateA, stateBd) = J;
      Jmatrix(stateBd, stateA) = J;
    }
  }
  return Jmatrix;
}

Eigen::MatrixXd BSECoupling::Fulldiag(const Eigen::MatrixXd& J_dimer) {
  int bse_exc = _levA + _levB;
  Eigen::MatrixXd Jmat = Eigen::MatrixXd::Zero(bse_exc, bse_exc);
  int ct = ctAB.rows() + ctBA.rows();
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> es(J_dimer);
  if (tools::globals::verbose && bse_exc + ct < 10) {
    XTP_LOG(logDEBUG, *_pLog)
        << "---------------------------------------" << flush;
    XTP_LOG(logDEBUG, *_pLog) << "Eigenvectors of J" << flush;
    XTP_LOG(logDEBUG, *_pLog) << es.eigenvectors() << flush;
    XTP_LOG(logDEBUG, *_pLog) << "J_eigenvalues[Hrt]" << flush;
    XTP_LOG(logDEBUG, *_pLog) << es.eigenvalues() << flush;
    XTP_LOG(logDEBUG, *_pLog)
        << "---------------------------------------" << flush;
  }
  // Calculate projection on subspace for every pair of excitons separately
  for (int stateA = 0; stateA < _levA; stateA++) {
    for (int stateB = 0; stateB < _levB; stateB++) {
      int stateBd = stateB + _levA;
      XTP_LOG(logDEBUG, *_pLog)
          << TimeStamp() << "   Calculating coupling between exciton A"
          << stateA + 1 << " and exciton B" << stateB + 1 << flush;
      std::vector<int> index;
      std::vector<int> signvec;
      for (int i = 0; i < bse_exc + ct; i++) {
        if (i == int(stateA) || i == int(stateBd)) {
          double close = 0.0;
          int ind = 0;
          int sign = 0;
          // row
          for (int j = 0; j < bse_exc + ct; j++) {
            bool check = true;
            // if index i is already in index
            // should not happen but if one vector was similar to two others.
            for (unsigned l = 0; l < index.size(); l++) {
              if (j == index[l]) {
                check = false;
                break;
              }
            }
            if (check && std::abs(es.eigenvalues()(i, j)) > close) {
              ind = j;
              close = std::abs(es.eigenvalues()(i, j));
              if (es.eigenvalues()(i, j) >= 0) {
                sign = 1;
              } else {
                sign = -1;
              }
            }
          }
          index.push_back(ind);
          signvec.push_back(sign);
        }
      }

      XTP_LOG(logDEBUG, *_pLog)
          << TimeStamp() << "   Order is: [Initial state n->nth eigenvalue]"
          << flush;
      XTP_LOG(logDEBUG, *_pLog) << "    A" << stateA + 1 << ":" << stateA + 1
                                << "->" << index[0] + 1 << " ";
      XTP_LOG(logDEBUG, *_pLog) << "    B" << stateB + 1 << ":" << stateBd + 1
                                << "->" << index[1] + 1 << " " << flush;

      // setting up transformation matrix Tmat and diagonal matrix Emat for the
      // eigenvalues;
      Eigen::MatrixXd Emat = Eigen::MatrixXd::Zero(2, 2);
      Eigen::MatrixXd Tmat = Eigen::MatrixXd::Zero(2, 2);
      // find the eigenvectors which are most similar to the initial states
      // row
      for (int i = 0; i < 2; i++) {
        int k = index[i];
        double sign = signvec[i];
        double normr = 1 / std::sqrt(es.eigenvectors()(stateA, k) *
                                         es.eigenvectors()(stateA, k) +
                                     es.eigenvectors()(stateBd, k) *
                                         es.eigenvectors()(stateBd, k));
        Tmat(0, i) = sign * es.eigenvectors()(stateA, k) * normr;
        Tmat(1, i) = sign * es.eigenvectors()(stateBd, k) * normr;
        Emat(i, i) = es.eigenvectors()(k);
      }

      if ((Tmat(1, 1) * Tmat(0, 0) - Tmat(1, 0) * Tmat(0, 1)) < 0) {
        XTP_LOG(logDEBUG, *_pLog)
            << " Reduced state matrix is not in a right handed basis, "
               "multiplying second eigenvector by -1 "
            << flush;
        Tmat(0, 1) = -Tmat(0, 1);
        Tmat(1, 1) = -Tmat(1, 1);
      }

      if (tools::globals::verbose) {
        XTP_LOG(logDEBUG, *_pLog)
            << "---------------------------------------" << flush;
        XTP_LOG(logDEBUG, *_pLog) << "_T" << flush;
        XTP_LOG(logDEBUG, *_pLog) << Tmat << flush;
      }

      Eigen::MatrixXd S_small = Tmat * Tmat.transpose();
      if (tools::globals::verbose) {

        XTP_LOG(logDEBUG, *_pLog) << "S_small" << flush;
        XTP_LOG(logDEBUG, *_pLog) << S_small << flush;
      }
      // orthogonalize that matrix

      Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> ss(S_small);
      Eigen::MatrixXd sm1 = ss.operatorInverseSqrt();
      Emat = sm1 * Emat * sm1;

      XTP_LOG(logDEBUG, *_pLog)
          << TimeStamp() << "   Smallest value of dimer overlapmatrix is "
          << ss.eigenvalues()(0) << flush;
      if (tools::globals::verbose) {

        XTP_LOG(logDEBUG, *_pLog) << "S-1/2" << flush;
        XTP_LOG(logDEBUG, *_pLog) << sm1 << flush;
        XTP_LOG(logDEBUG, *_pLog) << "E_ortho" << flush;
        XTP_LOG(logDEBUG, *_pLog) << Emat << flush;
      }
      Tmat = Tmat * sm1;

      if (tools::globals::verbose) {

        XTP_LOG(logDEBUG, *_pLog) << "T_ortho" << flush;
        XTP_LOG(logDEBUG, *_pLog) << Tmat << flush;
        XTP_LOG(logDEBUG, *_pLog)
            << "---------------------------------------" << flush;
      }

      Eigen::MatrixXd J_small = Tmat * Emat * Tmat.transpose();
      if (tools::globals::verbose) {
        XTP_LOG(logDEBUG, *_pLog) << "T_ortho*E_ortho*T_ortho^T" << flush;
        XTP_LOG(logDEBUG, *_pLog) << J_small << flush;
      }

      Jmat(stateA, stateBd) = J_small(0, 1);
      Jmat(stateBd, stateA) = J_small(1, 0);
    }
  }

  return Jmat;
}

}  // namespace xtp
}  // namespace votca
