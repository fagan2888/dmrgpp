#include "String.h"
const PsimagLite::String license=
"Copyright (c) 2009-2012 , UT-Battelle, LLC\n"
"All rights reserved\n"
"\n"
"[DMRG++, Version 2.0.0]\n"
"\n"
"*********************************************************\n"
"THE SOFTWARE IS SUPPLIED BY THE COPYRIGHT HOLDERS AND\n"
"CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED\n"
"WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
"WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A\n"
"PARTICULAR PURPOSE ARE DISCLAIMED.\n"
"\n"
"Please see full open source license included in file LICENSE.\n"
"*********************************************************\n"
"\n";

#ifndef USE_FLOAT
typedef double RealType;
#else
typedef float RealType;
#endif

#include <unistd.h>
#include "Observer.h"
#include "ObservableLibrary.h"
#include "IoSimple.h"
#include "Operators.h"
#include "Concurrency.h"
#include "Geometry/Geometry.h" 
#include "CrsMatrix.h"
#include "ModelHelperLocal.h"
#include "ModelHelperSu2.h"
#include "InternalProductOnTheFly.h"
#include "VectorWithOffset.h"
#include "VectorWithOffsets.h"
#include "GroundStateTargetting.h"
#include "DmrgSolver.h" // only used for types
#include "TimeStepTargetting.h"
#include "DynamicTargetting.h"
#include "AdaptiveDynamicTargetting.h"
#include "CorrectionTargetting.h"
#include "MettsTargetting.h"
#include "BasisWithOperators.h"
#include "LeftRightSuper.h"
#include "InputNg.h"
#include "Provenance.h"
#include "InputCheck.h"
#include "ModelSelector.h"

using namespace Dmrg;

typedef std::complex<RealType> ComplexType;

typedef  PsimagLite::CrsMatrix<ComplexType> MySparseMatrixComplex;
typedef  PsimagLite::CrsMatrix<RealType> MySparseMatrixReal;
typedef  PsimagLite::Geometry<RealType,ProgramGlobals> GeometryType;
typedef PsimagLite::IoSimple::In IoInputType;
typedef PsimagLite::InputNg<InputCheck> InputNgType;
typedef ParametersDmrgSolver<RealType,InputNgType::Readable> DmrgSolverParametersType;

struct OperatorOptions {
	
	OperatorOptions()
	    : site(0),dof(0),label(""),fermionicSign(0),transpose(false)
	{}
	
	SizeType site;
	SizeType dof;
	PsimagLite::String label;
	int fermionicSign;
	bool transpose;
};

template<template<typename> class ModelHelperTemplate,
         template<typename> class VectorWithOffsetTemplate,
         typename MySparseMatrix>
void mainLoop(GeometryType& geometry,
              const PsimagLite::String& targetting,
              InputNgType::Readable& io,
              const DmrgSolverParametersType& params,
              const OperatorOptions& obsOptions)
{
	typedef Basis<MySparseMatrix> BasisType;
	typedef Operators<BasisType> OperatorsType;
	typedef typename OperatorsType::OperatorType OperatorType;
	typedef BasisWithOperators<OperatorsType> BasisWithOperatorsType;
	typedef LeftRightSuper<BasisWithOperatorsType,BasisType> LeftRightSuperType;
	typedef ModelHelperTemplate<LeftRightSuperType> ModelHelperType;
	typedef ModelBase<ModelHelperType,
	                  DmrgSolverParametersType,
	                  InputNgType::Readable,
	                  GeometryType> ModelBaseType;
	typedef typename MySparseMatrix::value_type ComplexOrRealType;
	typedef VectorWithOffsetTemplate<ComplexOrRealType> VectorWithOffsetType;
	typedef std::pair<SizeType,SizeType> PairType;
	typedef typename ModelHelperType::SparseElementType SparseElementType;
	typedef PsimagLite::Matrix<SparseElementType> MatrixType;
	
	ModelSelector<ModelBaseType> modelSelector(params.model);
	const ModelBaseType& model = modelSelector(params,io,geometry);

	MatrixType opC = model.naturalOperator(obsOptions.label,obsOptions.site,obsOptions.dof);
	std::cout<<"#label="<<obsOptions.label<<" site="<<obsOptions.site<<" dof="<<obsOptions.dof<<"\n";

	Su2Related su2Related;

	MatrixType opC2;
	if (obsOptions.transpose)
		transposeConjugate(opC2,opC);

	OperatorType opC3((obsOptions.transpose) ? opC2 : opC,
	                  obsOptions.fermionicSign,
	                  PairType(0,0),
	                  1,
	                  su2Related);

	opC3.save(std::cout);
}

void usage(const char* name)
{
	std::cerr<<"USAGE is "<<name<<" -f filename -F ";
	std::cerr<<"fermionicSign -l label [-d dof] [-s site] [-t]\n";
}

int main(int argc,char *argv[])
{
	typedef PsimagLite::Concurrency ConcurrencyType;

	using namespace Dmrg;

	PsimagLite::String filename="";
	OperatorOptions options;
	int opt = 0;
	while ((opt = getopt(argc, argv,"f:s:l:d:F:t")) != -1) {
		switch (opt) {
		case 'f':
			filename = optarg;
			break;
		case 's':
			options.site = atoi(optarg);
			break;
		case 'l':
			options.label = optarg;
			break;
		case 'd':
			options.dof = atoi(optarg);
			break;
		case 't':
			options.transpose = true;
			break;
		case 'F':
			options.fermionicSign = atoi(optarg);
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	//sanity checks here
	if (filename=="" || options.label=="" || options.fermionicSign == 0) {
		usage(argv[0]);
		return 1;
	}

	ConcurrencyType concurrency(&argc,&argv,1);
	
	// print license
	if (ConcurrencyType::root()) {
		std::cerr<<license;
		Provenance provenance;
		std::cout<<provenance;
	}

	//Setup the Geometry
	InputCheck inputCheck;
	InputNgType::Writeable ioWriteable(filename,inputCheck);
	InputNgType::Readable io(ioWriteable);
	GeometryType geometry(io);

	//! Read the parameters for this run
	//ParametersModelType mp(io);
	DmrgSolverParametersType dmrgSolverParams(io);


	bool su2=false;
	if (dmrgSolverParams.options.find("useSu2Symmetry")!=PsimagLite::String::npos) su2=true;

	PsimagLite::String targetting=inputCheck.getTargeting(dmrgSolverParams.options);
	
	if (targetting!="GroundStateTargetting" && su2) throw PsimagLite::RuntimeError("SU(2)"
 		" supports only GroundStateTargetting for now (sorry!)\n");

	if (su2) {
		if (dmrgSolverParams.targetQuantumNumbers[2]>0) { 
			mainLoop<ModelHelperSu2,VectorWithOffsets,MySparseMatrixReal>
			(geometry,targetting,io,dmrgSolverParams,options);
		} else {
			mainLoop<ModelHelperSu2,VectorWithOffset,MySparseMatrixReal>
			(geometry,targetting,io,dmrgSolverParams,options);
		}
		return 0;
	}

	if (targetting=="GroundStateTargetting") {
		mainLoop<ModelHelperLocal,VectorWithOffset,MySparseMatrixReal>
		(geometry,targetting,io,dmrgSolverParams, options);
	} else if (targetting=="TimeStepTargetting") {
		mainLoop<ModelHelperLocal,VectorWithOffsets,MySparseMatrixComplex>
		(geometry,targetting,io,dmrgSolverParams, options);
	} else {
		mainLoop<ModelHelperLocal,VectorWithOffsets,MySparseMatrixReal>
		(geometry,targetting,io,dmrgSolverParams, options);
	}
} // main

