/*
 * Copyright 2009-2019 The VOTCA Development Team (http://www.votca.org)
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

#pragma once
#ifndef __VOTCA_DIPOLEDIPOLEINTERACTION_H
#define __VOTCA_DIPOLEDIPOLEINTERACTION_H
#include <votca/xtp/eeinteractor.h>
#include <votca/xtp/eigen.h>
namespace votca {
namespace xtp {
class DipoleDipoleInteraction;
}
}  // namespace votca
namespace Eigen {
namespace internal {
// MatrixReplacement looks-like a SparseMatrix, so let's inherits its traits:
template <>
struct traits<votca::xtp::DipoleDipoleInteraction>
    : public Eigen::internal::traits<Eigen::MatrixXd> {};
}  // namespace internal
}  // namespace Eigen

namespace votca {
namespace xtp {

class DipoleDipoleInteraction
    : public Eigen::EigenBase<DipoleDipoleInteraction> {
 public:
  // Required typedefs, constants, and method:
  typedef double Scalar;
  typedef double RealScalar;
  typedef int StorageIndex;
  enum {
    ColsAtCompileTime = Eigen::Dynamic,
    MaxColsAtCompileTime = Eigen::Dynamic,
    IsRowMajor = false
  };

  DipoleDipoleInteraction(const eeInteractor& interactor,
                          const std::vector<PolarSegment>& segs)
      : _interactor(interactor) {
    _size = 0;
    for (const PolarSegment seg : segs) {
      _size += 3 * seg.size();
    }
    _sites.reserve(_size / 3);
    for (const PolarSegment& seg : segs) {
      for (const PolarSite& site : seg) {
        _sites.push_back(&site);
      }
    }
  }

  Index rows() const { return this->_size; }
  Index cols() const { return this->_size; }

  template <typename Vtype>
  Eigen::Product<DipoleDipoleInteraction, Vtype, Eigen::AliasFreeProduct>
      operator*(const Eigen::MatrixBase<Vtype>& x) const {
    return Eigen::Product<DipoleDipoleInteraction, Vtype,
                          Eigen::AliasFreeProduct>(*this, x.derived());
  }

  Eigen::Matrix3Xd ThreeRows(int index) const {
    const PolarSite& site1 = *_sites[index];
    Eigen::Matrix3Xd result = Eigen::Matrix3Xd(3, _size);
    for (int i = 0; i < _size / 3; i++) {
      if (i == index) {
        result.block<3, 3>(0, 3 * i) = site1.getPInv();
      } else {
        const PolarSite& site2 = *_sites[i];
        result.block<3, 3>(0, 3 * i) =
            _interactor.FillTholeInteraction_diponly(site1, site2);
      }
    }
    return result;
  }

 private:
  const eeInteractor& _interactor;
  std::vector<const PolarSite*> _sites;
  int _size;
};
}  // namespace xtp
}  // namespace votca

namespace Eigen {

namespace internal {

// replacement of the mat*vect operation
template <typename Vtype>
struct generic_product_impl<votca::xtp::DipoleDipoleInteraction, Vtype,
                            DenseShape, DenseShape, GemvProduct>
    : generic_product_impl_base<
          votca::xtp::DipoleDipoleInteraction, Vtype,
          generic_product_impl<votca::xtp::DipoleDipoleInteraction, Vtype>> {

  typedef typename Product<votca::xtp::DipoleDipoleInteraction, Vtype>::Scalar
      Scalar;

  template <typename Dest>
  static void scaleAndAddTo(Dest& dst,
                            const votca::xtp::DipoleDipoleInteraction& op,
                            const Vtype& v, const Scalar& alpha) {
    // returns dst = alpha * op * v
    // alpha must be 1 here
    assert(alpha == Scalar(1) && "scaling is not implemented");
    EIGEN_ONLY_USED_FOR_DEBUG(alpha);
    int iterations = op.rows() / 3;
    // make the mat vect product
    //#pragma omp parallel for
    for (int i = 0; i < iterations; i++) {
      const Eigen::Vector3d result = op.ThreeRows(i) * v;
      dst(3 * i) = result.x();
      dst(3 * i + 1) = result.y();
      dst(3 * i + 2) = result.z();
    }
  }
};

}  // namespace internal
}  // namespace Eigen

#endif  //__VOTCA_DIPOLEDIPOLEINTERACTION_H