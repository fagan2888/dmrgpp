/*
Copyright (c) 2009-2015, UT-Battelle, LLC
All rights reserved

[DMRG++, Version 3.0]
[by G.A., Oak Ridge National Laboratory]

UT Battelle Open Source Software License 11242008

OPEN SOURCE LICENSE

Subject to the conditions of this License, each
contributor to this software hereby grants, free of
charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), a
perpetual, worldwide, non-exclusive, no-charge,
royalty-free, irrevocable copyright license to use, copy,
modify, merge, publish, distribute, and/or sublicense
copies of the Software.

1. Redistributions of Software must retain the above
copyright and license notices, this list of conditions,
and the following disclaimer.  Changes or modifications
to, or derivative works of, the Software should be noted
with comments and the contributor and organization's
name.

2. Neither the names of UT-Battelle, LLC or the
Department of Energy nor the names of the Software
contributors may be used to endorse or promote products
derived from this software without specific prior written
permission of UT-Battelle.

3. The software and the end-user documentation included
with the redistribution, with or without modification,
must include the following acknowledgment:

"This product includes software produced by UT-Battelle,
LLC under Contract No. DE-AC05-00OR22725  with the
Department of Energy."

*********************************************************
DISCLAIMER

THE SOFTWARE IS SUPPLIED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER, CONTRIBUTORS, UNITED STATES GOVERNMENT,
OR THE UNITED STATES DEPARTMENT OF ENERGY BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

NEITHER THE UNITED STATES GOVERNMENT, NOR THE UNITED
STATES DEPARTMENT OF ENERGY, NOR THE COPYRIGHT OWNER, NOR
ANY OF THEIR EMPLOYEES, REPRESENTS THAT THE USE OF ANY
INFORMATION, DATA, APPARATUS, PRODUCT, OR PROCESS
DISCLOSED WOULD NOT INFRINGE PRIVATELY OWNED RIGHTS.

*********************************************************

*/
/** \ingroup DMRG */
/*@{*/

/*! \file HeisenbergAncillaC.h
 *
 *  An implementation of the Quantum Heisenberg Model to use with  DmrgSolver
 *
 */

#ifndef DMRG_HEISENBERG_ANCILLAC_H
#define DMRG_HEISENBERG_ANCILLAC_H

#include <algorithm>
#include "ModelBase.h"
#include "ParametersHeisenbergAncillaC.h"
#include "LinkProductHeisenbergAncillaC.h"
#include "CrsMatrix.h"
#include "VerySparseMatrix.h"
#include "SpinSquaredHelper.h"
#include "SpinSquared.h"
#include "ProgramGlobals.h"
#include "ModelCommon.h"

namespace Dmrg {

/* Order of the basis
  For S=1/2 and NUMBER_OF_ORBITALS=2 it is
  down A, down B; down A, up B; up A, down B; up A, up B;
  */
template<typename ModelBaseType>
class HeisenbergAncillaC : public ModelBaseType {

public:

	typedef typename ModelBaseType::ModelHelperType ModelHelperType;
	typedef typename ModelHelperType::BasisType BasisType;
	typedef typename ModelBaseType::GeometryType GeometryType;
	typedef typename ModelBaseType::LeftRightSuperType LeftRightSuperType;
	typedef typename ModelBaseType::LinkProductStructType LinkProductStructType;
	typedef typename ModelBaseType::LinkType LinkType;
	typedef typename ModelHelperType::OperatorsType OperatorsType;
	typedef typename ModelHelperType::RealType RealType;
	typedef	typename ModelBaseType::VectorType VectorType;
	typedef	typename std::pair<SizeType,SizeType> PairSizeType;

private:

	typedef typename ModelBaseType::BlockType BlockType;
	typedef typename ModelBaseType::SolverParamsType SolverParamsType;
	typedef typename ModelHelperType::SparseMatrixType SparseMatrixType;
	typedef typename SparseMatrixType::value_type SparseElementType;
	typedef unsigned int long long WordType;
	typedef LinkProductHeisenbergAncillaC<ModelHelperType> LinkProductType;
	typedef ModelCommon<ModelBaseType,LinkProductType> ModelCommonType;
	typedef typename ModelBaseType::InputValidatorType InputValidatorType;
	typedef PsimagLite::Matrix<SparseElementType> MatrixType;
	typedef typename PsimagLite::Vector<SizeType>::Type VectorSizeType;
	typedef typename ModelBaseType::VectorRealType VectorRealType;

	static const int NUMBER_OF_ORBITALS=2;

public:

	typedef typename PsimagLite::Vector<unsigned int long long>::Type HilbertBasisType;
	typedef typename OperatorsType::OperatorType OperatorType;
	typedef typename OperatorType::PairType PairType;
	typedef typename PsimagLite::Vector<OperatorType>::Type VectorOperatorType;
	typedef	typename ModelBaseType::MyBasis MyBasis;
	typedef	typename ModelBaseType::BasisWithOperatorsType MyBasisWithOperators;
	typedef typename MyBasis::BasisDataType BasisDataType;

	HeisenbergAncillaC(const SolverParamsType& solverParams,
	                  InputValidatorType& io,
	                  GeometryType const &geometry)
	    : ModelBaseType(io,new ModelCommonType(solverParams,geometry)),
	      modelParameters_(io),
	      geometry_(geometry)
	{
		SizeType n = geometry_.numberOfSites();
		SizeType m = modelParameters_.magneticField.size();
		if (m > 0 && m != n) {
			PsimagLite::String msg("HeisenbergAncillaC: If provided, ");
			msg += " MagneticField must be a vector of " + ttos(n) + " entries.\n";
			throw PsimagLite::RuntimeError(msg);
		}

		if (BasisType::useSu2Symmetry()) {
			PsimagLite::String msg("HeisenbergAncillaC: SU(2) symmetry");
			msg += " is not implemented yet.\n";
			throw PsimagLite::RuntimeError(msg);
		}

		if (modelParameters_.twiceTheSpin != 1) {
			PsimagLite::String msg("HeisenbergAncillaC: spin > 1/2");
			msg += " HIGHLY EXPERIMENTAL!\n";
			std::cout<<msg;
		}
	}

	SizeType memResolv(PsimagLite::MemResolv&,
	                   SizeType,
	                   PsimagLite::String = "") const
	{
		return 0;
	}

	void print(std::ostream& os) const { os<<modelParameters_; }

	SizeType hilbertSize(SizeType) const
	{
		return pow(modelParameters_.twiceTheSpin + 1, NUMBER_OF_ORBITALS);
	}

	//! find  operator matrices for (i,sigma) in the natural basis,
	//! find quantum numbers and number of electrons
	//! for each state in the basis
	void setNaturalBasis(VectorOperatorType& operatorMatrices,
	                     SparseMatrixType &hamiltonian,
	                     BasisDataType &q,
	                     const BlockType& block,
	                     const RealType& time) const
	{
		HilbertBasisType natBasis;

		VectorSizeType qvector;
		setNaturalBasis(natBasis,qvector,block);

		setOperatorMatrices(operatorMatrices,block);

		setSymmetryRelated(q,natBasis,block.size());

		this->calcHamiltonian(hamiltonian,operatorMatrices,block,time);
	}

	//! set operator matrices for sites in block
	void setOperatorMatrices(VectorOperatorType& operatorMatrices,
	                         const BlockType& block) const
	{
		if (block.size() != 1) {
			PsimagLite::String msg("HeisenbergAncillaC: only blocks");
			msg += " of size 1 can be added for now.\n";
			throw PsimagLite::RuntimeError(msg);
		}

		HilbertBasisType natBasis;
		SparseMatrixType tmpMatrix;

		VectorSizeType qvector;
		setNaturalBasis(natBasis,qvector,block);

		operatorMatrices.clear();
		for (SizeType i=0;i<block.size();i++) {
			// Set the operators S^+_i for orbital a in the natural basis
			tmpMatrix=findSplusMatrices(i,natBasis);

			typename OperatorType::Su2RelatedType su2related;
			su2related.source.push_back(i*2);
			su2related.source.push_back(i*2+NUMBER_OF_ORBITALS);
			su2related.source.push_back(i*2);
			su2related.transpose.push_back(-1);
			su2related.transpose.push_back(-1);
			su2related.transpose.push_back(1);
			su2related.offset = NUMBER_OF_ORBITALS;

			OperatorType myOp(tmpMatrix,1,PairType(2,2),-1,su2related);
			operatorMatrices.push_back(myOp);

			// Set the operators S^z_i in the natural basis
			tmpMatrix = findSzMatrices(i,natBasis);
			typename OperatorType::Su2RelatedType su2related2;
			OperatorType myOp2(tmpMatrix,1,PairType(2,1),1.0/sqrt(2.0),su2related2);
			operatorMatrices.push_back(myOp2);

			// Set the operators \Delta_i in the natural basis
			tmpMatrix = findDeltaMatrices(i,natBasis);
			typename OperatorType::Su2RelatedType su2related3;
			OperatorType myOp3(tmpMatrix,1,PairType(0,0),1.0,su2related3);
			operatorMatrices.push_back(myOp3);
		}
	}

	MatrixType naturalOperator(const PsimagLite::String& what,
	                           SizeType site,
	                           SizeType) const
	{
		BlockType block;
		block.resize(1);
		block[0]=site;
		typename PsimagLite::Vector<OperatorType>::Type creationMatrix;
		setOperatorMatrices(creationMatrix,block);
		assert(creationMatrix.size()>0);
		SizeType nrow = creationMatrix[0].data.row();

		if (what == "i" || what=="identity") {
			MatrixType tmp(nrow,nrow);
			for (SizeType i = 0; i < tmp.n_row(); ++i) tmp(i,i) = 1.0;
			return tmp;
		}

		if (what=="+") { // S^+
			MatrixType tmp;
			crsMatrixToFullMatrix(tmp,creationMatrix[0].data);
			return tmp;
		}

		if (what=="-") { // S^-
			MatrixType tmp;
			crsMatrixToFullMatrix(tmp,creationMatrix[0].data);
			return transposeConjugate(tmp);
		}

		if (what=="z") { // S^z
			MatrixType tmp;
			crsMatrixToFullMatrix(tmp,creationMatrix[1].data);
			return tmp;
		}

		if (what=="d") { // \Delta
			MatrixType tmp;
			crsMatrixToFullMatrix(tmp,creationMatrix[2].data);
			return tmp;
		}

		throw std::logic_error("DmrgObserve::spinOperator(): invalid argument\n");
	}

	//! find all states in the natural basis for a block of n sites
	void setNaturalBasis(HilbertBasisType& basis,
	                     VectorSizeType& q,
	                     const VectorSizeType& block) const
	{
		assert(block.size()==1);
		SizeType total = hilbertSize(block[0]);

		for (SizeType i=0;i<total;i++) basis.push_back(i);
		BasisDataType qq;
		setSymmetryRelated(qq,basis,block.size());
		MyBasis::findQuantumNumbers(q,qq);
	}

	//! Dummy since this model has no fermion sign
	void findElectrons(VectorSizeType& electrons,
	                   const HilbertBasisType& basis,
	                   SizeType) const
	{
		electrons.resize(basis.size());
		for (SizeType i=0;i<electrons.size();i++)
			electrons[i] = 0;
	}

	void addDiagonalsInNaturalBasis(SparseMatrixType &hmatrix,
	                                const VectorOperatorType& cm,
	                                const BlockType& block,
	                                RealType,
	                                RealType factorForDiagonals=1.0)  const
	{
		SizeType linSize = geometry_.numberOfSites();
		if (modelParameters_.magneticField.size() != linSize) return;

		SizeType n=block.size();
		SparseMatrixType tmpMatrix;

		for (SizeType i=0;i<n;i++) {
			// magnetic field
			RealType tmp = modelParameters_.magneticField[block[i]]*factorForDiagonals;
			multiplyScalar(tmpMatrix,cm[1+i*2].data,tmp);

			hmatrix += tmpMatrix;
		}
	}

	virtual void setTargetNumbers(VectorSizeType& t,
	                              SizeType sites,
	                              SizeType direction) const
	{
		modelParameters_.targetQuantum.setTargetNumbers(t,
		                                                sites,
		                                                geometry_.numberOfSites(),
		                                                direction);
	}

private:

	//! Find S^+_i in the natural basis natBasis
	SparseMatrixType findSplusMatrices(int,const HilbertBasisType& natBasis) const
	{
		SizeType total = natBasis.size();
		MatrixType cm(total,total);
		RealType j = 0.5*modelParameters_.twiceTheSpin;
		SizeType total1 = modelParameters_.twiceTheSpin + 1;
		for (SizeType ii=0;ii<total;ii++) {
			PairSizeType ket = getOneOrbital(natBasis[ii]);

			SizeType bra1 = ket.first + 1;
			if (bra1 >= total1) continue;
			PairSizeType bra(bra1,ket.second);
			SizeType jj = getFullIndex(bra);
			RealType m = ket.first - j;
			RealType x = j*(j+1)-m*(m+1);
			assert(x>=0);
			cm(ii,jj) = sqrt(x);
		}

		SparseMatrixType operatorMatrix(cm);
		return operatorMatrix;
	}

	//! Find S^z_i in the natural basis natBasis
	SparseMatrixType findSzMatrices(int,const HilbertBasisType& natBasis) const
	{
		SizeType total = natBasis.size();
		MatrixType cm(total,total);
		RealType j = 0.5*modelParameters_.twiceTheSpin;

		for (SizeType ii=0;ii<total;ii++) {
			PairSizeType ket = getOneOrbital(natBasis[ii]);

			RealType m = ket.first - j;
			cm(ii,ii) = m;
		}

		SparseMatrixType operatorMatrix(cm);
		return operatorMatrix;
	}

	SparseMatrixType findDeltaMatrices(int,const HilbertBasisType& natBasis) const
	{
		SizeType total = natBasis.size();
		MatrixType cm(total,total);
		RealType j = 0.5*modelParameters_.twiceTheSpin;
		SizeType total1 = modelParameters_.twiceTheSpin + 1;
		for (SizeType ii=0;ii<total;ii++) {
			PairSizeType ket = getOneOrbital(natBasis[ii]);

			SizeType bra1 = ket.first + 1;
			if (bra1 >= total1) continue;

			SizeType bra2 = ket.second + 1;
			if (bra2 >= total1) continue;

			PairSizeType bra(bra1,bra2);
			SizeType jj = getFullIndex(bra);
			RealType m = ket.first - j;
			RealType x1 = j*(j+1)-m*(m+1);
			assert(x1>=0);
			m = ket.second - j;
			RealType x2 = j*(j+1)-m*(m+1);
			assert(x2>=0);
			cm(ii,jj) = sqrt(x1)*sqrt(x2);
		}

		SparseMatrixType operatorMatrix(cm);
		return operatorMatrix;
	}

	void setSymmetryRelated(BasisDataType& q,
	                        const HilbertBasisType& basis,
	                        int n) const
	{
		if (n!=1)
			PsimagLite::RuntimeError("HeisenbergAncillaC::setSymmetryRelated n=1 only\n");

		// find j,m and flavors (do it by hand since we assume n==1)
		// note: we use 2j instead of j
		// note: we use m+j instead of m
		// This assures us that both j and m are SizeType
		typedef std::pair<SizeType,SizeType> PairType;
		typename PsimagLite::Vector<PairType>::Type jmvalues;
		VectorSizeType flavors;
		PairType jmSaved;
		jmSaved.first = modelParameters_.twiceTheSpin;
		jmSaved.second = basis[0];
		jmSaved.first++;
		jmSaved.second++;

		q.szPlusConst.resize(basis.size());
		q.electrons.resize(basis.size());
		for (SizeType i=0;i<basis.size();i++) {
			PairType jmpair;
			jmpair.first = modelParameters_.twiceTheSpin;
			jmpair.second = basis[i];
			jmvalues.push_back(jmpair);
			PairSizeType ket = getOneOrbital(basis[i]);
			q.szPlusConst[i] = ket.first + ket.second;
			q.electrons[i] = NUMBER_OF_ORBITALS;
			flavors.push_back(1);
			jmSaved = jmpair;
		}

		q.jmValues = jmvalues;
		q.flavors = flavors;
	}

	PairSizeType getOneOrbital(SizeType state) const
	{
		SizeType total1 = modelParameters_.twiceTheSpin + 1;
		assert(NUMBER_OF_ORBITALS == 2);
		SizeType first = static_cast<SizeType>(state/total1);
		SizeType second = state % total1;
		return PairSizeType(first, second);
	}

	SizeType getFullIndex(PairSizeType p) const
	{
		return p.first*(modelParameters_.twiceTheSpin + 1) + p.second;
	}

	//serializr start class HeisenbergAncillaC
	//serializr vptr
	//serializr normal modelParameters_
	ParametersHeisenbergAncillaC<RealType>  modelParameters_;
	//serializr ref geometry_ start
	GeometryType const &geometry_;
}; // class HeisenbergAncillaC

} // namespace Dmrg
/*@}*/
#endif //DMRG_HEISENBERG_ANCILLAC_H

