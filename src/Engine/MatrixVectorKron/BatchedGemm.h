#ifndef BATCHEDGEMM_H
#define BATCHEDGEMM_H
#include <complex>
#include "Vector.h"

typedef long int BatchedDgemmIntegerType;
typedef PsimagLite::Vector<BatchedDgemmIntegerType>::Type VectorBatchedDgemmIntegerType;
typedef double BatchedDgemmFpType;
typedef std::complex<BatchedDgemmFpType> BatchedDgemmComplexType;

/*** BEGIN Functions symbols that need to be provided by PLUGIN_SC */

// All INPUTS except for Abatch and Bbatch
extern "C" void setup_vbatch(BatchedDgemmIntegerType, // number of connections
                             BatchedDgemmIntegerType, // number of patches
                             BatchedDgemmIntegerType*, // permutation left
                             BatchedDgemmIntegerType*, // permutation right
                             BatchedDgemmIntegerType*, // summed offsets
                             BatchedDgemmFpType**, // Abatch for library internal use (OUTPUT)
                             BatchedDgemmFpType**, // Bbatch for library internal use (OUTPUT)
                             BatchedDgemmFpType**, // Matrices A
                             BatchedDgemmIntegerType*, // Rows of Matrices A
                             BatchedDgemmFpType**, // Matrices B
                             BatchedDgemmIntegerType*); // Rows of Matrices B

// All INPUTS except for vout, Abatch and Bbatch are for library internal use
extern "C" void apply_Htarget_vbatch(BatchedDgemmIntegerType, // number of connections
                                     BatchedDgemmIntegerType, // number of patches
                                     BatchedDgemmIntegerType*, // permutation left
                                     BatchedDgemmIntegerType*, // permutation right
                                     BatchedDgemmIntegerType*, // summed offsets
                                     BatchedDgemmFpType*, // Abatch for library internal use
                                     BatchedDgemmFpType*, // Bbatch for library internal use
                                     BatchedDgemmFpType*, // vin, already copied in
                                     BatchedDgemmFpType*); // vout, will be copied out (OUTPUT)

// SEE setup_vbatch above
extern "C" void setup_vbatchCmplx(BatchedDgemmIntegerType,
                                  BatchedDgemmIntegerType,
                                  BatchedDgemmIntegerType*,
                                  BatchedDgemmIntegerType*,
                                  BatchedDgemmIntegerType*,
                                  BatchedDgemmComplexType**,
                                  BatchedDgemmComplexType**,
                                  BatchedDgemmComplexType**,
                                  BatchedDgemmIntegerType*,
                                  BatchedDgemmComplexType**,
                                  BatchedDgemmIntegerType*);

// SEE apply_Htarget_vbatch above
extern "C" void apply_Htarget_vbatchCmplx(BatchedDgemmIntegerType,
                                          BatchedDgemmIntegerType,
                                          BatchedDgemmIntegerType*,
                                          BatchedDgemmIntegerType*,
                                          BatchedDgemmIntegerType*,
                                          BatchedDgemmComplexType*,
                                          BatchedDgemmComplexType*,
                                          BatchedDgemmComplexType*,
                                          BatchedDgemmComplexType*);

// All OUTPUTS
extern "C" void unsetup_vbatch(BatchedDgemmFpType**, // Abatch for library internal use
                               BatchedDgemmFpType**); // B batch for library internal use

// SEE unsetup_vbatch above
extern "C" void unsetup_vbatchCmplx(BatchedDgemmComplexType**,
                                    BatchedDgemmComplexType**);

/*** END Functions symbols that need to be provided by PLUGIN_SC */

template<typename T>
void applyHtargetVbatch(BatchedDgemmIntegerType,
                        BatchedDgemmIntegerType,
                        const VectorBatchedDgemmIntegerType&,
                        const VectorBatchedDgemmIntegerType&,
                        const VectorBatchedDgemmIntegerType&,
                        T*,
                        T*,
                        T*,
                        T*);

template<>
inline void applyHtargetVbatch<BatchedDgemmComplexType>(BatchedDgemmIntegerType a,
                                                        BatchedDgemmIntegerType b,
                                                        const VectorBatchedDgemmIntegerType& c,
                                                        const VectorBatchedDgemmIntegerType& d,
                                                        const VectorBatchedDgemmIntegerType& e,
                                                        BatchedDgemmComplexType* f,
                                                        BatchedDgemmComplexType* g,
                                                        BatchedDgemmComplexType* h,
                                                        BatchedDgemmComplexType* i)
{
	BatchedDgemmIntegerType* cptr = const_cast<BatchedDgemmIntegerType*>(&(c[0]));
	BatchedDgemmIntegerType* dptr = const_cast<BatchedDgemmIntegerType*>(&(d[0]));
	BatchedDgemmIntegerType* eptr = const_cast<BatchedDgemmIntegerType*>(&(e[0]));
	apply_Htarget_vbatchCmplx(a, b, cptr, dptr, eptr, f, g, h, i);
}

template<>
inline void applyHtargetVbatch<BatchedDgemmFpType>(BatchedDgemmIntegerType a,
                                                   BatchedDgemmIntegerType b,
                                                   const VectorBatchedDgemmIntegerType& c,
                                                   const VectorBatchedDgemmIntegerType& d,
                                                   const VectorBatchedDgemmIntegerType& e,
                                                   BatchedDgemmFpType* f,
                                                   BatchedDgemmFpType* g,
                                                   BatchedDgemmFpType* h,
                                                   BatchedDgemmFpType* i)
{
	BatchedDgemmIntegerType* cptr = const_cast<BatchedDgemmIntegerType*>(&(c[0]));
	BatchedDgemmIntegerType* dptr = const_cast<BatchedDgemmIntegerType*>(&(d[0]));
	BatchedDgemmIntegerType* eptr = const_cast<BatchedDgemmIntegerType*>(&(e[0]));
	apply_Htarget_vbatch(a, b, cptr, dptr, eptr, f, g, h, i);
}

/******/

template<typename T>
void setupVbatch(BatchedDgemmIntegerType,
                 BatchedDgemmIntegerType,
                 const VectorBatchedDgemmIntegerType&,
                 const VectorBatchedDgemmIntegerType&,
                 const VectorBatchedDgemmIntegerType&,
                 T**,
                 T**,
                 T**,
                 const VectorBatchedDgemmIntegerType&,
                 T**,
                 const VectorBatchedDgemmIntegerType&);

template<>
inline void setupVbatch<BatchedDgemmComplexType>(BatchedDgemmIntegerType a,
                                                 BatchedDgemmIntegerType b,
                                                 const VectorBatchedDgemmIntegerType& c,
                                                 const VectorBatchedDgemmIntegerType& d,
                                                 const VectorBatchedDgemmIntegerType& e,
                                                 BatchedDgemmComplexType** f,
                                                 BatchedDgemmComplexType** g,
                                                 BatchedDgemmComplexType** h,
                                                 const VectorBatchedDgemmIntegerType& i,
                                                 BatchedDgemmComplexType** j,
                                                 const VectorBatchedDgemmIntegerType& k)
{
	BatchedDgemmIntegerType* cptr = const_cast<BatchedDgemmIntegerType*>(&(c[0]));
	BatchedDgemmIntegerType* dptr = const_cast<BatchedDgemmIntegerType*>(&(d[0]));
	BatchedDgemmIntegerType* eptr = const_cast<BatchedDgemmIntegerType*>(&(e[0]));
	BatchedDgemmIntegerType* iptr = const_cast<BatchedDgemmIntegerType*>(&(i[0]));
	BatchedDgemmIntegerType* kptr = const_cast<BatchedDgemmIntegerType*>(&(k[0]));
	setup_vbatchCmplx(a, b, cptr, dptr, eptr, f, g, h, iptr, j, kptr);
}

template<>
inline void setupVbatch<BatchedDgemmFpType>(BatchedDgemmIntegerType a,
                                            BatchedDgemmIntegerType b,
                                            const VectorBatchedDgemmIntegerType& c,
                                            const VectorBatchedDgemmIntegerType& d,
                                            const VectorBatchedDgemmIntegerType& e,
                                            BatchedDgemmFpType** f,
                                            BatchedDgemmFpType** g,
                                            BatchedDgemmFpType** h,
                                            const VectorBatchedDgemmIntegerType& i,
                                            BatchedDgemmFpType** j,
                                            const VectorBatchedDgemmIntegerType& k)
{
	BatchedDgemmIntegerType* cptr = const_cast<BatchedDgemmIntegerType*>(&(c[0]));
	BatchedDgemmIntegerType* dptr = const_cast<BatchedDgemmIntegerType*>(&(d[0]));
	BatchedDgemmIntegerType* eptr = const_cast<BatchedDgemmIntegerType*>(&(e[0]));
	BatchedDgemmIntegerType* iptr = const_cast<BatchedDgemmIntegerType*>(&(i[0]));
	BatchedDgemmIntegerType* kptr = const_cast<BatchedDgemmIntegerType*>(&(k[0]));
	setup_vbatch(a, b, cptr, dptr, eptr, f, g, h, iptr, j, kptr);
}

template<typename T>
void unsetupVbatch(T**, T**);

template<>
inline void unsetupVbatch<BatchedDgemmFpType>(BatchedDgemmFpType** f,
                                              BatchedDgemmFpType** g)
{
	unsetup_vbatch(f, g);
}

template<>
inline void unsetupVbatch<BatchedDgemmComplexType>(BatchedDgemmComplexType** f,
                                                   BatchedDgemmComplexType** g)
{
	unsetup_vbatchCmplx(f, g);
}

/******/

namespace Dmrg {

template<typename InitKronType>
class BatchedGemm {

	typedef typename InitKronType::ArrayOfMatStructType ArrayOfMatStructType;
	typedef typename ArrayOfMatStructType::MatrixDenseOrSparseType MatrixDenseOrSparseType;
	typedef typename MatrixDenseOrSparseType::VectorType VectorType;
	typedef typename InitKronType::SparseMatrixType SparseMatrixType;
	typedef typename SparseMatrixType::value_type ComplexOrRealType;
	typedef typename MatrixDenseOrSparseType::MatrixType MatrixType;
	typedef typename InitKronType::GenIjPatchType GenIjPatchType;
	typedef typename GenIjPatchType::BasisType BasisType;

	// Say 1 for 1-based, 0 for 0-based
	static const SizeType baseForIntegerVectors_ = 1;

public:

	BatchedGemm(const InitKronType& initKron)
	    :  initKron_(initKron), Abatch_(0), Bbatch_(0)
	{
		convertOffsets(offsets_);
		SizeType total = initKron_.numberOfPatches(InitKronType::OLD);
		SizeType nC = initKron_.connections();
		ComplexOrRealType** aptr = new ComplexOrRealType*[total*total*nC];
		ComplexOrRealType** bptr = new ComplexOrRealType*[total*total*nC];
		VectorBatchedDgemmIntegerType ldAptr(total*total*nC);
		VectorBatchedDgemmIntegerType ldBptr(total*total*nC);

		for (SizeType outPatch = 0; outPatch < total; ++outPatch) {
			for (SizeType inPatch = 0; inPatch < total; ++inPatch) {
				for (SizeType ic=0;ic<nC;++ic) {
					const ArrayOfMatStructType& xiStruct = initKron_.xc(ic);
					const ArrayOfMatStructType& yiStruct = initKron_.yc(ic);

					const MatrixDenseOrSparseType& Amat =  xiStruct(outPatch,inPatch);
					const MatrixDenseOrSparseType& Bmat =  yiStruct(outPatch,inPatch);

					const MatrixType& AmatDense = Amat.dense();
					const MatrixType& BmatDense = Bmat.dense();

					ComplexOrRealType* a = const_cast<ComplexOrRealType*>(&(AmatDense(0,0)));
					ComplexOrRealType* b = const_cast<ComplexOrRealType*>(&(BmatDense(0,0)));
					aptr[inPatch + outPatch*total + ic*total*total] = a;
					bptr[inPatch + outPatch*total + ic*total*total] = b;

					ldAptr[inPatch + outPatch*total + ic*total*total] = AmatDense.rows();
					ldBptr[inPatch + outPatch*total + ic*total*total] = BmatDense.rows();
				}
			}
		}

		// call setup_vbatch and fill aBatch_ and bBatch_

		convertToVector(pLeft_, initKron_.lrs(InitKronType::NEW).left());
		convertToVector(pRight_, initKron_.lrs(InitKronType::NEW).right());

		setupVbatch(initKron.connections(),
		            initKron_.patch(InitKronType::NEW, GenIjPatchType::LEFT).size(),
		            pLeft_,
		            pRight_,
		            offsets_,
		            &Abatch_,
		            &Bbatch_,
		            aptr,
		            ldAptr,
		            bptr,
		            ldBptr);

		delete[] aptr;
		aptr = 0;
		delete[] bptr;
		bptr = 0;
	}

	~BatchedGemm()
	{
		unsetupVbatch(&Abatch_, &Bbatch_);
	}

	bool enabled() const { return true; }

	void matrixVector(VectorType& vout, const VectorType& vin) const
	{
		ComplexOrRealType* vinptr = const_cast<ComplexOrRealType*>(&(vin[0]));
		ComplexOrRealType* voutptr = const_cast<ComplexOrRealType*>(&(vout[0]));
		applyHtargetVbatch(initKron_.connections(),
		                   initKron_.patch(InitKronType::NEW, GenIjPatchType::LEFT).size(),
		                   pLeft_,
		                   pRight_,
		                   offsets_,
		                   Abatch_,
		                   Bbatch_,
		                   vinptr,
		                   voutptr);
	}

private:

	void convertToVector(VectorBatchedDgemmIntegerType& v, const BasisType& b) const
	{
		SizeType total = b.partition();
		v.clear();
		v.resize(total + baseForIntegerVectors_, 0);
		for (SizeType i = 0; i < total; ++i)
			v[i + baseForIntegerVectors_] = b.partition(i);
	}

	void convertOffsets(VectorBatchedDgemmIntegerType& v) const
	{
		SizeType total = initKron_.offsetForPatches(InitKronType::NEW);
		v.clear();
		v.resize(total + baseForIntegerVectors_, 0);
		for (SizeType i = 0; i < total; ++i)
			v[i + baseForIntegerVectors_] =initKron_.offsetForPatches(InitKronType::NEW, i);
	}

	BatchedGemm(const BatchedGemm&);

	BatchedGemm& operator=(const BatchedGemm&);

	const InitKronType& initKron_;
	VectorBatchedDgemmIntegerType offsets_;
	VectorBatchedDgemmIntegerType pLeft_;
	VectorBatchedDgemmIntegerType pRight_;
	ComplexOrRealType* Abatch_;
	ComplexOrRealType* Bbatch_;
};
}
#endif // BATCHEDGEMM_H
