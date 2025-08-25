
/*
Linear Prediction DLL module for Prospa
Author: Cameron Dykstra
Address: dykstra.cameron@gmail.com

Uses Eigen, lapacke, and CppNumericalSolvers
https://github.com/PatWie/CppNumericalSolvers
*/



#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <random>
#include <complex>
#include <Eigen/Core>
#include <Eigen/Dense>

#define MATLAB
#include <cppoptlib/meta.h>
#include <cppoptlib/problem.h>
#include <cppoptlib/solver/neldermeadsolver.h>

#define lapack_complex_float std::complex<float>
#define lapack_complex_double std::complex<double>
#include <Lapack/lapacke.h>

#include "../Global files/includesDLL.h"


#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

using namespace cppoptlib;
using namespace Eigen;

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
short PhaseCorrect(DLLParameters*, char* args);
short FastLinearPredict(DLLParameters*, char *args);
short LinearPredict(DLLParameters*, char* args);
short EigenTest(DLLParameters*, char *args);

MatrixXcf RRFcf(MatrixXcf& A, int l);

char** parList; // Parameter list - built up by pp commands
long szList;    // Number of entries in parameter list

std::default_random_engine gen;
std::normal_distribution<float> gaussian(0.0, 1.0);

std::stringstream fmt;

/*******************************************************************************
Extension procedure to add commands to Prospa
********************************************************************************/

EXPORT short  AddCommands(char* command, char* parameters, DLLParameters* dpar)
{
	short r = RETURN_FROM_DLL;

	//if (!strcmp(command, "linearpredict")) r = LinearPredict(dpar, parameters);
	if (!strcmp(command, "fastlinearpredict")) r = FastLinearPredict(dpar, parameters);
	if (!strcmp(command, "eigensvd")) r = EigenTest(dpar, parameters);
	//if (!strcmp(command, "phasecorrect"))  r = PhaseCorrect(dpar, parameters);

	return(r);
}

/*******************************************************************************
Extension procedure to list commands in DLL
********************************************************************************/

EXPORT void  ListCommands(void)
{
	TextMessage("\n\n   Fast Linear Prediction DLL module\n\n");
	//TextMessage("   linearpredict ...... Applies Linear Prediction to the input vector (slow but good for low SNR)\n");
	TextMessage("   fastlinearpredict .. Applies Linear Prediction to the input vector (faster but requires good SNR)\n");
	TextMessage("   eigensvd  .......... Performs SVD on input matrix m = U*diag(S')*V')\n");
	//TextMessage("   phasecorrect  .. Corrects the phase of the input spectrum\n");
}

/*******************************************************************************
Extension procedure to return syntax in DLL
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
	syntax[0] = '\0';
	//if (!strcmp(cmd, "linearpredict"))           strcpy(syntax, "VEC y = linearpredict(VEC FID, INT numCoefficients, INT maxRank, INT padBack, INT padForward)");
	if (!strcmp(cmd, "fastlinearpredict"))       strcpy(syntax, "VEC y = fastlinearpredict(VEC FID, INT numCoefficients, INT maxRank, INT padBack, INT padForward)");
	if (!strcmp(cmd, "eigensvd"))                strcpy(syntax, "(MAT S, U,  V) = eigensvd(MAT m)");
	//if (!strcmp(cmd, "phasecorrect"))            strcpy(syntax, "VEC y, FLOAT ph0, FLOAT ph1 = phasecorrect(VEC spectrum, SHORT[0 or 1] order)");

	if (syntax[0] == '\0')
		return(false);
	return(true);
}


ArrayXXcf ProspaVar2EigenXXcf(Variable* prosVar)
{
	int N = prosVar->GetDimX();
	long rows = prosVar->GetDimY();
	long cols = prosVar->GetDimX();
	complex** arrayIn = prosVar->GetCMatrix2D();

	ArrayXXcf arrayOut = ArrayXXcf(rows, cols);

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			arrayOut(y, x).real(arrayIn[y][x].r);
			arrayOut(y, x).imag(arrayIn[y][x].i);
		}
	}
	return(arrayOut);
}

ArrayXXcf Prospa2EigenXXcf(complex** arrayIn, int cols, int rows)
{
	ArrayXXcf arrayOut = ArrayXXcf(rows, cols);

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			arrayOut(y, x).real(arrayIn[y][x].r);
			arrayOut(y, x).imag(arrayIn[y][x].i);
		}
	}
	return(arrayOut);
}

complex** Eigen2ProspaXXcf(ArrayXXcf arrayIn)
{
	int cols = arrayIn.cols();
	int rows = arrayIn.rows();

	complex** arrayOut = MakeCMatrix2D(cols, rows);

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			arrayOut[y][x].r = arrayIn(y, x).real();
			arrayOut[y][x].i = arrayIn(y, x).imag();
		}
	}

	return(arrayOut);
}

ArrayXXf ProspaVar2EigenXXf(Variable* prosVar)
{
	int N = prosVar->GetDimX();
	long rows = prosVar->GetDimY();
	long cols = prosVar->GetDimX();
	float** arrayIn = prosVar->GetMatrix2D();

	ArrayXXf arrayOut = ArrayXXf(rows, cols);

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			arrayOut(y, x) = arrayIn[y][x];
		}
	}
	return(arrayOut);
}


float** Eigen2ProspaXXf(ArrayXXf arrayIn)
{
	int cols = arrayIn.cols();
	int rows = arrayIn.rows();

	float** arrayOut = MakeMatrix2D(cols, rows);

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			arrayOut[y][x] = arrayIn(y, x);
		}
	}

	return(arrayOut);
}
//
//class PhaseCorrectProblem : public Problem<double>
//{
//public:
//	short order = 1;
//	float value_init = 0;
//	float ph1Penalty = 0.01;
//	float threshold_mult = 3;
//	Vector2d ph_init;
//	ArrayXcd S;
//
//	using typename cppoptlib::Problem<double>::TVector; // ADDED
//
//	PhaseCorrectProblem(ArrayXcd spectrum, short _order) 
//   {
//		order = _order;
//		S = spectrum;
//		ph_init = Vector2d(std::arg(spectrum(0))-PI/8,PI/4);
//		value_init = value(ph_init);
//	}
//
//	
//	double value(const TVector& x)
//	{
//		ArrayXd ph(S.size());
//		ph.fill(x(0));
//		if (order > 0)
//      { // Generate a phase array with p0 offset and p1 slope
//			for (int i = 0; i < S.size(); i++)
//			{
//				ph(i) += (x(1)*i)/S.size();
//			}
//		}
//		// Multiply the spectrum by this phase shift
//		ArrayXcd PS = S*Eigen::exp(std::complex<double>(0, -1)*ph);
//		ArrayXd W(PS.size() - 1);
//		// Calculate the differential
//		for (int i = 1; i < S.size(); i++)// diff
//		{
//			W(i - 1) = (PS(i) - PS(i - 1)).real();
//		}
//		// Work out the 3 sigma level for the RMS
//		double threshold = threshold_mult * std::sqrt(W.square().mean()); 
//		double last = 0;
//		for (int i = 0; i < W.size(); i++) // soft threshold and integrate
//		{
//			double val = W(i);
//			if (val > threshold)
//         {
//				val -= threshold;
//			}
//			else if (val < -threshold)
//         {
//				val += threshold;
//			}
//			else 
//         {
//				val = 0;
//			}
//			W(i) = last + val;
//			last = W(i);
//		}
//		//mean of squares, with a penalty to keep ph1 small
//		return W.square().mean() + std::abs(x(1)*value_init*ph1Penalty);
//	}
//};
////
//short PhaseCorrect(DLLParameters* par, char *args)
//{
//	short nrArgs;
//	Variable SVar;
//	short order;
//	Eigen::VectorXd ph; // CHANGED
//
//	nrArgs = ArgScan(par->itfc, args, 2, "spectrum,order", "ee", "vd", &SVar, &order);
//	if (nrArgs < 0)
//		return(nrArgs);
//
//	if (SVar.GetType() != CMATRIX2D) {
//		ErrorMessage("First argument to 'phasecorrect' should be a complex matrix");
//		return(ERR);
//	}
//
//	int N = SVar.GetDimX();
//	long rows = VarRowSize(&SVar);
//	long cols = VarColSize(&SVar);
//
//	complex** arrayIn = SVar.GetCMatrix2D();
//	ArrayXXcf spectrum = Prospa2EigenXXcf(arrayIn, N, 1);
//
//	PhaseCorrectProblem P(spectrum.row(0).cast<std::complex<double> >(), order);
//	NelderMeadSolver<PhaseCorrectProblem> solver;
//	if (order > 0)
//	{
//		ph = P.ph_init;
//	}
//	else
//	{
//		ph = P.ph_init.head(1);
//	}
//
//	solver.minimize(P, ph);
//
//	if (order == 0)
//   {
//		ph.conservativeResize(2);
//		ph(1) = 0;
//	}
//
//	while (ph(0) > PI/2)
//   {
//		ph(0) -= PI;
//	}
//
//	while (ph(0) < -PI/2)
//   {
//		ph(0) += PI;
//	}
//	
//	// apply the phase correction
//	ArrayXf phases(spectrum.size());
//	phases.fill(ph(0));
//	for (int i = 0; i < spectrum.size(); i++)
//	{
//		phases(i) += (ph(1)*i) / spectrum.size();
//	}
//	spectrum.row(0) = spectrum.row(0)*(std::complex<float>(0, -1)*phases).exp().transpose();
//
//	// Allocate space for the output matrix
//	complex** arrayOut = Eigen2ProspaXXcf(spectrum);
//
//	par->retVar[1].MakeAndLoadCMatrix2D(arrayOut, N, 1);
//	FreeCMatrix2D(arrayOut);
//
//	par->retVar[2].MakeAndSetFloat((float)ph(0));
//	par->retVar[3].MakeAndSetFloat((float)ph(1));
//
//	par->nrRetVar = 3;
//
//	return(OK);
//}


short EigenTest(DLLParameters* par, char* args)
{
	std::stringstream buffer;

	//MatrixXd m(2, 2);
	//m(0, 0) = 3;
	//m(1, 0) = 2.5;
	//m(0, 1) = -1;
	//m(1, 1) = m(1, 0) + m(0, 1);
	//buffer << m << std::endl;
	//TextMessage("\n\n%s", buffer.str().c_str());

   short nrArgs;
	Variable prosVar1;
	Variable prosVar2;

	nrArgs = ArgScan(par->itfc, args, 1, "mat1,mat2", "ee", "vv", &prosVar1, &prosVar2);
	if (nrArgs < 0)
		return(nrArgs);


	if (prosVar1.GetType() == CMATRIX2D)
	{
		ArrayXXcf spectrum = ProspaVar2EigenXXcf(&prosVar1);

		//buffer << spectrum << std::endl;
	  // TextMessage("\n\n%s", buffer.str().c_str());

		spectrum = spectrum * 2;

		complex** arrayOut = Eigen2ProspaXXcf(spectrum);

		int rows = spectrum.rows();
		int cols = spectrum.cols();

		par->retVar[1].AssignCMatrix2D(arrayOut, cols, rows);
		par->nrRetVar = 1;

	}
	else if (prosVar1.GetType() == MATRIX2D)
	{
		ArrayXXf mat1 = ProspaVar2EigenXXf(&prosVar1);
		ArrayXXf mat2 = ProspaVar2EigenXXf(&prosVar2);

		//buffer << spectrum << std::endl;
	  // TextMessage("\n\n%s", buffer.str().c_str());

		//MatrixXf result = mat1.matrix()*mat2.matrix();

		//std::cout << "A: " << mat1.matrix() << std::endl;

		Eigen::JacobiSVD<Eigen::MatrixXf> svd(mat1.matrix(), Eigen::ComputeFullU | Eigen::ComputeFullV);
		//std::cout << "U: " << svd.matrixU() << std::endl;;
		//std::cout << "V: " << svd.matrixV() << std::endl;;
		//std::cout << "V: " << svd.singularValues() << std::endl;;

		float** singularValues = Eigen2ProspaXXf(svd.singularValues().array());
		float** UValues = Eigen2ProspaXXf(svd.matrixU().array());
		float** VValues = Eigen2ProspaXXf(svd.matrixV().array());

		int rows = svd.singularValues().array().rows();
		int cols = svd.singularValues().array().cols();
		par->retVar[1].AssignMatrix2D(singularValues, cols, rows);
		rows = svd.matrixU().array().rows();
		cols = svd.matrixU().array().cols();
		par->retVar[2].AssignMatrix2D(UValues, cols, rows);
		rows = svd.matrixV().array().rows();
		cols = svd.matrixV().array().cols();
		par->retVar[3].AssignMatrix2D(VValues, cols, rows);
		par->nrRetVar = 3;

	}

	return(OK);
}


// A simpler version than Cameron's which is faster but more noise sensitive

short FastLinearPredict(DLLParameters* par, char *args)
{
	short nrArgs;
	Variable FIDVar;
	long sizeFID, nrCoeff, sigPoints, padBack = 0, padForward = 0;
	long newSize;

	nrArgs = ArgScan(par->itfc, args, 5, "FID, sigPoints, nrCoeff, padBack, padForward", "eeeee", "vllll", &FIDVar, &sigPoints, &nrCoeff, &padBack, &padForward);
	if (nrArgs < 0)
		return(nrArgs);

	if (FIDVar.GetType() != CMATRIX2D) 
	{
		ErrorMessage("First argument to 'linearpredict' should be a complex matrix");
		return(ERR);
	}

	if (padBack < 0 || padForward < 0) 
	{
		ErrorMessage("padBack and padForward must be positive or zero");
		return(ERR);
	}

	if(sigPoints - nrCoeff < 1)
	{
		ErrorMessage("number of coefficients less than the number of significant points");
		return(ERR);
	}

	sizeFID = FIDVar.GetDimX();

	if(sizeFID < sigPoints)
	{
		ErrorMessage("Data set must be at least %d points long.", sigPoints);
		return(ERR);
	}

	long rows = VarRowSize(&FIDVar);
	long cols = VarColSize(&FIDVar);

	complex** arrayIn = FIDVar.GetCMatrix2D();


	// Store signal
	ArrayXXcf signal(1, sizeFID);
	for (int i = 0; i < sizeFID; i++)
	{
		signal(0, i) = std::complex<float>(arrayIn[0][i].r, arrayIn[0][i].i);
	}

	ArrayXXcf newSignal;

	try
	{

		// Model the data with : A c = y, where
		// A is the shifted data matrix (M x N)
		// y is the part of the data used for the prediction (M)
		// c are the coefficients describing the LP (to be determined) (N)

		if (padBack > 0)
		{
			//std::cout << "Signal size: " << signal.rows() << "," << signal.cols() << std::endl;
			//std::cout << "Signal: " << signal.matrix() << std::endl;

			// Set up A: A(i .. i+M,j .. j+N) = signal(i+j+1)
			MatrixXcf A(sigPoints - nrCoeff, nrCoeff);
			for (int i = 0; i < sigPoints - nrCoeff; i++)
			{
				A.row(i) = signal.row(0).segment(i + 1, (int)nrCoeff);
			}

			//std::cout << "A size: " << A.rows() << "," << A.cols() << std::endl;
			//std::cout << "A: " << A.matrix() << std::endl;


			// Set up y: y(i .. i+M) = signal(i ... i+M)
			VectorXcf y(sigPoints - nrCoeff);
			y = signal.row(0).segment(0, sigPoints - nrCoeff);

			//std::cout << "y size: " << y.rows() << "," << y.cols() << std::endl;
			//std::cout << "y: " << y.matrix() << std::endl;


			// Use SVD solver to solve a in a least square method.
			JacobiSVD<MatrixXcf> svd(A, ComputeThinU | ComputeThinV);
			VectorXcf coeff(nrCoeff); // c(0 ... N-1)
			coeff = svd.solve(y);

			//std::cout << "U: " << svd.matrixU() << std::endl;
			//std::cout << "V: " << svd.matrixV() << std::endl;
			//std::cout << "S: " << svd.singularValues() << std::endl;

			//std::cout << "coeff size: " << coeff.rows() << "," << coeff.cols() << std::endl;

			//std::cout << "coeff size: " << coeff.rows() << "," << coeff.cols() << std::endl;
			//std::cout << "coeff: " << coeff.matrix() << std::endl;

			// Create new signal newSignal = zeros[padBack], signal
			newSize = signal.size() + padBack;
			newSignal.resize(1, newSize);
			newSignal.block(0, padBack, 1, signal.size()) = signal;

			//std::cout << "newSignalIn: " << newSignal.matrix() << std::endl;

			// Predict earlier points using y(i-1) = SUM_j(c(j) * y(i+j)
			for (int i = padBack - 1; i >= 0; i--)
			{
				//	std::cout << "loop: " << newSignal.matrix().row(0).segment(i + 1, nrCoeff) << std::endl;
				newSignal(0, i) = coeff.conjugate().dot(newSignal.matrix().row(0).segment(i + 1, nrCoeff));
			}

			//std::cout << "newSignalOut: " << newSignal.matrix() << std::endl;

		}
		else if (padForward > 0)
		{
			// Set up A:
			MatrixXcf A(sigPoints - nrCoeff, nrCoeff);
			for (int i = 0; i < sigPoints - nrCoeff; i++)
			{
				A.row(i) = signal.row(0).segment(i, (int)nrCoeff);
			}

			// Set up y:
			VectorXcf y(sigPoints - nrCoeff);
			y = signal.row(0).segment(nrCoeff, sigPoints - nrCoeff);

			// Use SVD solver to solve a in a least square method.
			JacobiSVD<MatrixXcf> svd(A, ComputeThinU | ComputeThinV);
			VectorXcf coeff(nrCoeff);
			coeff = svd.solve(y);

			// Create new signal
			int newSize = signal.size() + padForward;
			newSignal.resize(1, newSize);
			newSignal.block(0, padForward, 1, signal.size()) = signal;

			// Predict later points
			for (int i = newSize - padForward; i < newSize; i++)
			{
				newSignal(0, i) = coeff.conjugate().dot(newSignal.matrix().row(0).segment(i - nrCoeff, nrCoeff));
			}
		}

		// Copy to prospa file
		complex** arrayOut = MakeCMatrix2D(newSize, 1);
		for (int i = 0; i < newSize; i++)
		{
			arrayOut[0][i].r = newSignal(0, i).real();
			arrayOut[0][i].i = newSignal(0, i).imag();
		}

		par->retVar[1].AssignCMatrix2D(arrayOut, newSize, 1);

		par->nrRetVar = 1;

	}
	catch (char* errStr)
	{
		ErrorMessage("Linear prediction failed");
		par->nrRetVar = 0;
		return(ERR);
	}

	return(OK);
}

// Cameron's original version - slower but works better with noisy signals.

short LinearPredict(DLLParameters* par, char* args)
{
	short nrArgs;
	Variable FIDVar;
	long N, M, R, padBack, padForward;

	int P = 10; //oversampling param

	nrArgs = ArgScan(par->itfc, args, 5, "FID,numCoeff,maxRank,padBack,padForward", "eeeee", "vllll", &FIDVar, &M, &R, &padBack, &padForward);
	if (nrArgs < 0)
		return(nrArgs);

	if (FIDVar.GetType() != CMATRIX2D) {
		ErrorMessage("First argument to 'linearpredict' should be a complex matrix");
		return(ERR);
	}

	if (padBack < 0 || padForward < 0) {
		ErrorMessage("padBack and padForward must be positive or zero");
		return(ERR);
	}

	N = FIDVar.GetDimX();
	long rows = VarRowSize(&FIDVar);
	long cols = VarColSize(&FIDVar);

	complex** arrayIn = FIDVar.GetCMatrix2D();

	ArrayXXcf signal = Prospa2EigenXXcf(arrayIn, N, 1);

	//use maximum rank if it is 0 or too big
	if (R <= 0 || R > MIN(N, N - M)) {
		R = MIN(N, N - M);
		fmt.str("");
		fmt << "Info: Rank set to " << R << std::endl;
		TextMessage((char*)fmt.str().c_str());
	}

	// set up the padded signal
	long new_N = N + padBack + padForward;
	ArrayXXcf new_signal(1, new_N);
	new_signal.block(0, padBack, 1, N) = signal;

	// pad backward:
	if (padBack > 0) {
		// Xa = x
		// set up X: 700ms for N=2^14, M=3/4 N, R=16
		MatrixXcf X(N - M, M);
		for (int i = 0; i < N - M; i++)
		{
			X.row(i) = signal.row(0).segment(i + 1, (int)M);
		}

		//set up x:
		VectorXcf x(N - M);
		x = signal.row(0).segment(0, N - M);

		// Partial SVD (see http://arxiv.org/pdf/0909.4061v2.pdf alg 5.1)
		MatrixXcf Q = RRFcf(X, R); // 600 ms for N=2^14, M=3/4 N, R=16
		MatrixXcf B = Q.adjoint() * X; // 600 ms for N=2^14, M=3/4 N, R=16
		VectorXf s(R);
		MatrixXcf U(R, R);
		MatrixXcf Vt(R, M);
		VectorXf superb(M);
		int result = LAPACKE_cgesvd(LAPACK_COL_MAJOR, 'S', 'S', R, M, B.data(), (int)B.rows(), s.data(), U.data(), (int)U.rows(), Vt.data(), (int)Vt.rows(), superb.data()); // 100 ms for N=2^14, M=3/4 N, R=16
		if (result != 0) {
			fmt.str("");
			fmt << "ERROR: LAPACK CGESVD returned " << result;
			ErrorMessage((char*)fmt.str().c_str());
			return(ERR);
		}
		//float Q_err = (X - Q * (Q.adjoint() * X)).norm()/X.norm();
		//fmt.str("");
		//fmt << "Info: Q_err: " << Q_err << std::endl;
		//TextMessage((char *)fmt.str().c_str());

		// invert singular values
		VectorXcf s_Inv(R);
		for (int i = 0; i < R; i++)
		{
			s_Inv(i) = 1 / s(i);
		}

		// build a = V S^-1 U* x
		VectorXcf a(M);
		a = Vt.adjoint() * (s_Inv.asDiagonal() * (U.adjoint() * (Q.adjoint() * x)));

		// do the prediction
		for (int i = padBack - 1; i >= 0; i--)
		{
			new_signal(0, i) = a.conjugate().dot(new_signal.matrix().row(0).segment(i + 1, M));
		}
	}

	//pad forward
	if (padForward > 0) {
		MatrixXcf X(N - M, M);
		for (int i = 0; i < N - M; i++)
		{
			X.row(i) = signal.row(0).segment(i, (int)M);
		}

		VectorXcf x(N - M);
		x = signal.row(0).segment(M, N - M);

		MatrixXcf Q = RRFcf(X, R);
		MatrixXcf B = Q.adjoint() * X;
		VectorXf s(R);
		MatrixXcf U(R, R);
		MatrixXcf Vt(R, M);
		VectorXf superb(M);
		int result = LAPACKE_cgesvd(LAPACK_COL_MAJOR, 'S', 'S', R, M, B.data(), (int)B.rows(), s.data(), U.data(), (int)U.rows(), Vt.data(), (int)Vt.rows(), superb.data());
		if (result != 0) {
			fmt.str("");
			fmt << "ERROR: LAPACK CGESVD returned " << result;
			ErrorMessage((char*)fmt.str().c_str());
			return(ERR);
		}

		VectorXcf s_Inv(R);
		for (int i = 0; i < R; i++)
		{
			s_Inv(i) = 1 / s(i);
		}

		VectorXcf a(M);
		a = Vt.adjoint() * (s_Inv.asDiagonal() * (U.adjoint() * (Q.adjoint() * x)));

		for (int i = new_N - padForward; i < new_N; i++)// 600 ms for 2^14
		{
			new_signal(0, i) = a.conjugate().dot(new_signal.matrix().row(0).segment(i - M, M));
		}
	}

	// Allocate space for the output matrix
	complex** arrayOut = Eigen2ProspaXXcf(new_signal);

	par->retVar[1].MakeAndLoadCMatrix2D(arrayOut, new_N, 1);
	FreeCMatrix2D(arrayOut);

	par->nrRetVar = 1;

	return(OK);
}

inline void gaussianMXcf(MatrixXcf& M) {
	for (int i = 0; i < M.rows(); ++i) {
		for (int j = 0; j < M.cols(); ++j) {
			M(i, j) = { gaussian(gen), gaussian(gen) };
		}
	}
}

MatrixXcf RRFcf(MatrixXcf& A, int l) {// http://arxiv.org/pdf/0909.4061v2.pdf Alg 4.1
	int m = A.rows(), n = A.cols();
	MatrixXcf O(n, l);
	gaussianMXcf(O);
	MatrixXcf Y = A * O;
	VectorXcf TAU(l);
	LAPACKE_cgeqrf(LAPACK_COL_MAJOR, m, l, Y.data(), (int)Y.rows(), TAU.data()); // QR decomp
	LAPACKE_cungqr(LAPACK_COL_MAJOR, m, l, l, Y.data(), (int)Y.rows(), TAU.data()); // build Q from reflectors

	//see alg 4.4, this helps weight toward larger singular values
	MatrixXcf Yt = A.adjoint() * Y;
	LAPACKE_cgeqrf(LAPACK_COL_MAJOR, n, l, Yt.data(), (int)Yt.rows(), TAU.data());
	LAPACKE_cungqr(LAPACK_COL_MAJOR, n, l, l, Yt.data(), (int)Yt.rows(), TAU.data());
	Y = A * Yt;
	LAPACKE_cgeqrf(LAPACK_COL_MAJOR, m, l, Y.data(), (int)Y.rows(), TAU.data());
	LAPACKE_cungqr(LAPACK_COL_MAJOR, m, l, l, Y.data(), (int)Y.rows(), TAU.data());

	return Y;
}

//short LinearPredict(DLLParameters* par, char *args)
//{
//	short nrArgs;
//	Variable FIDVar;
//	long N, M, R, padBack, padForward;
//
//	nrArgs = ArgScan(par->itfc, args, 5, "FID,numCoeff,maxRank,padBack,padForward", "eeeee", "vllll", &FIDVar, &M, &R, &padBack, &padForward);
//	if (nrArgs < 0)
//		return(nrArgs);
//
//	if (FIDVar.GetType() != CMATRIX2D) {
//		ErrorMessage("First argument to 'linearpredict' should be a complex matrix");
//		return(ERR);
//	}
//
//	if (padBack < 0 || padForward < 0) {
//		ErrorMessage("padBack and padForward must be positive or zero");
//		return(ERR);
//	}
//
//	N = FIDVar.GetDimX();
//	long rows = VarRowSize(&FIDVar);
//	long cols = VarColSize(&FIDVar);
//
//	complex** arrayIn = FIDVar.GetCMatrix2D();
//
//	ArrayXXcf signal = Prospa2EigenXXcf(arrayIn, N, 1);
//
//	//use maximum rank if it is 0 or too big
//	if (R <= 0 || R>MIN(N, N - M)) {
//		R = MIN(N, N - M);
//		fmt.str("");
//		fmt << "Warning: Rank set to " << R << std::endl;
//		TextMessage((char *)fmt.str().c_str());
//	}
//
//	// Xa = x
//	// set up X:
//	MatrixXcf X_F(N - M, M);
//	MatrixXcf X_B(N - M, M);
//	for (int i = 0; i < N - M; i++)
//	{
//		X_F.row(i) = signal.row(0).segment(i, (int)M);
//		X_B.row(i) = signal.row(0).segment(i + 1, (int)M);
//	}
//
//	//set up x:
//	VectorXcf x_F(N - M);
//	VectorXcf x_B(N - M);
//	x_F = signal.row(0).segment(M, N - M);
//	x_B = signal.row(0).segment(0, N - M);
//
//	// LAPACK SVD
//	int result;
//	VectorXf s_F(MIN(N - M, M));
//	MatrixXcf U_F(N - M, MIN(N - M, M));
//	MatrixXcf Vt_F(MIN(N - M, M), M);
//	VectorXf superb(N);
//	result = LAPACKE_cgesvd(LAPACK_COL_MAJOR, 'S', 'S', N - M, M, X_F.data(), (int)X_F.rows(), s_F.data(), U_F.data(), (int)U_F.rows(), Vt_F.data(), (int)Vt_F.rows(), superb.data());
//	if (result != 0) {
//		fmt.str("");
//		fmt << "ERROR: LAPACK CGESVD returned " << result;
//		ErrorMessage((char *)fmt.str().c_str());
//		return(ERR);
//	}
//	//fmt.str("");
//	//fmt << "s Forward: " << std::endl << s_F.head(R).transpose() << std::endl;
//	//TextMessage((char *)fmt.str().c_str());
//	VectorXf s_B(MIN(N - M, M));
//	MatrixXcf U_B(N - M, MIN(N - M, M));
//	MatrixXcf Vt_B(MIN(N - M, M), M);
//	result = LAPACKE_cgesvd(LAPACK_COL_MAJOR, 'S', 'S', N - M, M, X_B.data(), (int)X_B.rows(), s_B.data(), U_B.data(), (int)U_B.rows(), Vt_B.data(), (int)Vt_B.rows(), superb.data());
//	if (result != 0) {
//		fmt.str("");
//		fmt << "ERROR: LAPACK CGESVD returned " << result;
//		ErrorMessage((char *)fmt.str().c_str());
//		return(ERR);
//	}
//	//fmt.str("");
//	//fmt << "s Back: " << std::endl << s_B.head(R).transpose() << std::endl;
//	//TextMessage((char *)fmt.str().c_str());
//
//	// build X^-1
//	VectorXcf s_Inv_F(M);
//	VectorXcf s_Inv_B(M);
//	for (int i = 0; i < M; i++)
//	{
//		if (i<R) {
//			s_Inv_F(i) = 1 / s_F(i);
//			s_Inv_B(i) = 1 / s_B(i);
//		}
//		else {
//			s_Inv_F(i) = 0;
//			s_Inv_B(i) = 0;
//		}
//	}
//
//	// build a = V S^-1 U* x
//	VectorXcf a_F(M);
//	VectorXcf a_B(M);
//
//	a_F = Vt_F.adjoint() * s_Inv_F.asDiagonal() * U_F.adjoint() * x_F;
//	a_B = Vt_B.adjoint() * s_Inv_B.asDiagonal() * U_B.adjoint() * x_B;
//
//	long new_N = N + padBack + padForward;
//
//	ArrayXXcf new_signal(1, new_N);
//	new_signal.block(0, padBack, 1, N) = signal;
//
//	for (int i = padBack - 1; i >= 0; i--)
//	{
//		new_signal(0, i) = a_B.conjugate().dot(new_signal.matrix().row(0).segment(i + 1, M));
//	}
//
//	for (int i = new_N - padForward; i < new_N; i++)
//	{
//		new_signal(0, i) = a_F.conjugate().dot(new_signal.matrix().row(0).segment(i - M, M));
//	}
//
//
//	//VectorXcf s_B(M);
//	////VectorXcf s_Inv_B_F(SVD_B_F.singularValues().size());
//	//VectorXcf s_B_B(SVD_B_B.singularValues().size());
//	//for (int i = 0; i < M; i++)
//	//{
//	//	s_B(i) = SVD_B.singularValues()(i);
//	//}
//	//for (int i = 0; i < SVD_B_B.singularValues().size(); i++)
//	//{
//	//	s_B_B(i) = SVD_B_B.singularValues()(i);
//	//}
//
//	//new_signal.row(0).segment(2 * R, R) = s_B.head(R);
//	//new_signal.row(0).segment(3 * R, s_B_B.size()) = s_B_B;
//	//new_signal.row(0).head(M) = a_F;
//
//	// Allocate space for the output matrix
//	complex** arrayOut = Eigen2ProspaXXcf(new_signal, new_N, 1);
//
//	par->retVar[1].MakeAndLoadCMatrix2D(arrayOut, new_N, 1);
//	FreeCMatrix2D(arrayOut);
//
//	par->nrRetVar = 1;
//
//	return(OK);
//}

//MatrixXcf ARRFcf(MatrixXcf A, float eps, int r, int max_rank) {// http://arxiv.org/pdf/0909.4061v2.pdf Alg 4.2
//	//eps: 1e-3
//	//r: 10
//	int m = A.rows(), n = A.cols();
//	//eps = eps / (10.0*std::sqrt(2.0 / std::_Pi));
//	//A = A / A.colwise().norm().minCoeff();
//	MatrixXcf I = MatrixXcf::Identity(m, m);
//	MatrixXcf w(n, 1);
//	MatrixXcf W(n, r);
//	gaussianMXcf(W); // fill with complex gaussian samples
//	MatrixXcf Y = A * W;
//	MatrixXcf Q(m, 1);
//	int j = 0;
//	float err = Y.rightCols(r).colwise().norm().maxCoeff();
//	float lasterr = 2 * err;
//	float err_ratio = lasterr / err - 1.0f;
//	while (err_ratio > eps && j<max_rank) {
//		if (j > 0) {
//			Y.col(j).applyOnTheLeft(I - Q * Q.adjoint());
//			Q.conservativeResize(NoChange, Q.cols() + 1);
//		}
//		Q.col(j) = Y.col(j).normalized();
//
//		W.conservativeResize(NoChange, W.cols() + 1);
//		gaussianMXcf(w);
//		W.rightCols(1) = w;
//
//		Y.conservativeResize(NoChange, Y.cols() + 1);
//		Y.col(j + r) = (I - Q * Q.adjoint()) * A * W.col(j + r);
//
//		for (int i = j + 1; i < j + r; i++) {
//			Y.col(i) = Y.col(i) - Q.col(j) * Q.col(j).dot(Y.col(i));
//		}
//
//		j++;
//		lasterr = err;
//		err = Y.rightCols(r).colwise().norm().maxCoeff();
//		err_ratio = std::abs(lasterr / err - 1.0f);
//	}
//	return Q;
//}

//
///*
//Linear Prediction DLL module for Prospa
//Author: Cameron Dykstra, simplified by Craig Eccles
//Address: dykstra.cameron@gmail.com
//
//Uses Eigen, lapacke, and CppNumericalSolvers
//https://github.com/PatWie/CppNumericalSolvers
//*/
//
//#define EIGEN_MPL2_ONLY
//
//#include <stdlib.h>
//#include <string.h>
//#include <iostream>
//#include <sstream>   
//#include <cmath>
//#include <random>
//#include <complex>
//#include <Eigen/Core>
//#include <Eigen/Dense>
//#include "../Global files/includesDLL.h"
//
//
//#ifndef MIN
//#define MIN(x,y) (((x) < (y)) ? (x) : (y))
//#endif
//
////using namespace cppoptlib;
//using namespace Eigen;
//
//// Locally defined procedure and global variables
//
//EXPORT short AddCommands(char*, char*, DLLParameters*);
//EXPORT void  ListCommands(void);
//EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
//short PhaseCorrect(DLLParameters*, char *args);
//short LinearPredict(DLLParameters*, char *args);
//short EigenTest(DLLParameters*, char *args);
//
//char **parList; // Parameter list - built up by pp commands
//long szList;    // Number of entries in parameter list
//
//std::default_random_engine gen;
//std::normal_distribution<float> gaussian(0.0, 1.0);
//
//std::stringstream fmt;
//
//
///*******************************************************************************
//Extension procedure to add commands to Prospa
//********************************************************************************/
//
//EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
//{
//	short r = RETURN_FROM_DLL;
//
//	if (!strcmp(command, "linearpredict")) r = LinearPredict(dpar, parameters);
//	if (!strcmp(command, "eigen")) r = EigenTest(dpar, parameters);
//	//if (!strcmp(command, "phasecorrect"))  r = PhaseCorrect(dpar, parameters);
//
//	return(r);
//}
//
///*******************************************************************************
//Extension procedure to list commands in DLL
//********************************************************************************/
//
//EXPORT void  ListCommands(void)
//{
//	TextMessage("\n\n   Eigen functions DLL module\n\n");
//	TextMessage("   linearpredict .. Applies Linear Prediction to the input vector\n");
//	//TextMessage("   fastlinearpredict .. Applies Linear Prediction to the input vector, faster, less accurate\n");
////	TextMessage("   phasecorrect  .. Corrects the phase of the input spectrum\n");
//	TextMessage("   eigen  .. For testing the eigen library\n");
//}
//
///*******************************************************************************
//Extension procedure to return syntax in DLL
//********************************************************************************/
//
//EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
//{
//	syntax[0] = '\0';
//	if (!strcmp(cmd, "linearpredict"))           strcpy(syntax, "VEC y = linearpredict(VEC FID, INT sigPoints, INT nrCoeff, INT padBack, INT padForward)");
//	//if (!strcmp(cmd, "phasecorrect"))            strcpy(syntax, "VEC y, FLOAT ph0, FLOAT ph1 = phasecorrect(VEC spectrum, SHORT[0 or 1] order)");
//	if (!strcmp(cmd, "eigentest"))               strcpy(syntax, "eigen()");
//
//	if (syntax[0] == '\0')
//		return(false);
//	return(true);
//}
//
//ArrayXXcf ProspaVar2EigenXXcf(Variable* prosVar)
//{
//	int N = prosVar->GetDimX();
//	long rows = prosVar->GetDimY();
//	long cols = prosVar->GetDimX();
//	complex** arrayIn = prosVar->GetCMatrix2D();
//
//	ArrayXXcf arrayOut = ArrayXXcf(rows, cols);
//
//	for (int y = 0; y < rows; y++)
//	{
//		for (int x = 0; x < cols; x++)
//		{
//			arrayOut(y, x).real(arrayIn[y][x].r);
//			arrayOut(y, x).imag(arrayIn[y][x].i);
//		}
//	}
//	return(arrayOut);
//}
//
//ArrayXXcf Prospa2EigenXXcf(complex** arrayIn, int cols, int rows)
//{
//	ArrayXXcf arrayOut = ArrayXXcf(rows, cols);
//
//	for (int y = 0; y < rows; y++)
//	{
//		for (int x = 0; x < cols; x++)
//		{
//			arrayOut(y, x).real(arrayIn[y][x].r);
//			arrayOut(y, x).imag(arrayIn[y][x].i);
//		}
//	}
//	return(arrayOut);
//}
//

//
//complex** Eigen2ProspaXXcf(ArrayXXcf arrayIn)
//{
//	int cols = arrayIn.cols();
//	int rows = arrayIn.rows();
//
//	complex** arrayOut = MakeCMatrix2D(cols, rows);
//
//	for (int y = 0; y < rows; y++)
//	{
//		for (int x = 0; x < cols; x++)
//		{
//			arrayOut[y][x].r = arrayIn(y, x).real();
//			arrayOut[y][x].i = arrayIn(y, x).imag();
//		}
//	}
//
//	return(arrayOut);
//}

//
//
//

//
//
////
////
//class PhaseCorrectProblem : public Problem<double>
//{
//public:
//	short order = 1;
//	float value_init = 0;
//	float ph1Penalty = 0.01;
//	float threshold_mult = 3;
//	Vector2d ph_init;
//	ArrayXcd S;
//
//	PhaseCorrectProblem(ArrayXcd spectrum, short _order) 
//   {
//		order = _order;
//		S = spectrum;
//		ph_init = Vector2d(std::arg(spectrum(0))-PI/8,PI/4);
//		value_init = value(ph_init);
//	}
//
//	double value(const Vector<double> &x)
//   {
//		ArrayXd ph(S.size());
//		ph.fill(x(0));
//		if (order > 0)
//      { // Generate a phase array with p0 offset and p1 slope
//			for (int i = 0; i < S.size(); i++)
//			{
//				ph(i) += (x(1)*i)/S.size();
//			}
//		}
//		// Multiply the spectrum by this phase shift
//		ArrayXcd PS = S*Eigen::exp(std::complex<double>(0, -1)*ph);
//		ArrayXd W(PS.size() - 1);
//		// Calculate the differential
//		for (int i = 1; i < S.size(); i++)// diff
//		{
//			W(i - 1) = (PS(i) - PS(i - 1)).real();
//		}
//		// Work out the 3 sigma level for the RMS
//		double threshold = threshold_mult * std::sqrt(W.square().mean()); 
//		double last = 0;
//		for (int i = 0; i < W.size(); i++) // soft threshold and integrate
//		{
//			double val = W(i);
//			if (val > threshold)
//         {
//				val -= threshold;
//			}
//			else if (val < -threshold)
//         {
//				val += threshold;
//			}
//			else 
//         {
//				val = 0;
//			}
//			W(i) = last + val;
//			last = W(i);
//		}
//		//mean of squares, with a penalty to keep ph1 small
//		return W.square().mean() + std::abs(x(1)*value_init*ph1Penalty);
//	}
//};
//
//short PhaseCorrect(DLLParameters* par, char *args)
//{
//	short nrArgs;
//	Variable SVar;
//	short order;
//	Vector<double> ph;
//
//	nrArgs = ArgScan(par->itfc, args, 2, "spectrum,order", "ee", "vd", &SVar, &order);
//	if (nrArgs < 0)
//		return(nrArgs);
//
//	if (SVar.GetType() != CMATRIX2D) {
//		ErrorMessage("First argument to 'phasecorrect' should be a complex matrix");
//		return(ERR);
//	}
//
//	int N = SVar.GetDimX();
//	long rows = VarRowSize(&SVar);
//	long cols = VarColSize(&SVar);
//
//	complex** arrayIn = SVar.GetCMatrix2D();
//	ArrayXXcf spectrum = Prospa2EigenXXcf(arrayIn, N, 1);
//
//	PhaseCorrectProblem P(spectrum.row(0).cast<std::complex<double> >(), order);
//	NelderMeadSolver<double> solver;
//	if (order > 0) {
//		ph = P.ph_init;
//	}
//	else {
//		ph = P.ph_init.head(1);
//	}
//
//	solver.minimize(P, ph);
//
//	if (order == 0)
//   {
//		ph.conservativeResize(2);
//		ph(1) = 0;
//	}
//
//	while (ph(0) > PI/2)
//   {
//		ph(0) -= PI;
//	}
//
//	while (ph(0) < -PI/2)
//   {
//		ph(0) += PI;
//	}
//	
//	// apply the phase correction
//	ArrayXf phases(spectrum.size());
//	phases.fill(ph(0));
//	for (int i = 0; i < spectrum.size(); i++)
//	{
//		phases(i) += (ph(1)*i) / spectrum.size();
//	}
//	spectrum.row(0) = spectrum.row(0)*(std::complex<float>(0, -1)*phases).exp().transpose();
//
//	// Allocate space for the output matrix
//	complex** arrayOut = Eigen2ProspaXXcf(spectrum, N, 1);
//
//	par->retVar[1].MakeAndLoadCMatrix2D(arrayOut, N, 1);
//	FreeCMatrix2D(arrayOut);
//
//	par->retVar[2].MakeAndSetFloat((float)ph(0));
//	par->retVar[3].MakeAndSetFloat((float)ph(1));
//
//	par->nrRetVar = 3;
//
//	return(OK);
//}
//
////short FastLinearPredict(DLLParameters* par, char *args)
////{
////	short nrArgs;
////	Variable FIDVar;
////	long N, M, R, padBack, padForward;
////
////	int P = 10; //oversampling param
////
////	nrArgs = ArgScan(par->itfc, args, 5, "FID,numCoeff,maxRank,padBack,padForward", "eeeee", "vllll", &FIDVar, &M, &R, &padBack, &padForward);
////	if (nrArgs < 0)
////		return(nrArgs);
////
////	if (FIDVar.GetType() != CMATRIX2D)
////   {
////		ErrorMessage("First argument to 'linearpredict' should be a complex matrix");
////		return(ERR);
////	}
////
////	if (padBack < 0 || padForward < 0)
////   {
////		ErrorMessage("padBack and padForward must be positive or zero");
////		return(ERR);
////	}
////
////	N = FIDVar.GetDimX();
////	long rows = VarRowSize(&FIDVar);
////	long cols = VarColSize(&FIDVar);
////
////	complex** arrayIn = FIDVar.GetCMatrix2D();
////
////	// Convert Prospa vector to Eigen array
////	ArrayXXcf signal = Prospa2EigenXXcf(arrayIn, N, 1);
////
////	//use maximum rank if it is 0 or too big
////	if (R <= 0 || R>MIN(N, N - M))
////   {
////		R = MIN(N, N - M);
////		fmt.str("");
////		fmt << "Info: Rank set to " << R << std::endl;
////		TextMessage((char *)fmt.str().c_str());
////	}
////
////	// set up the padded signal
////	long new_N = N + padBack + padForward;
////	ArrayXXcf new_signal(1, new_N);
////	new_signal.block(0, padBack, 1, N) = signal;
////
////	// pad backward:
////	if (padBack > 0)
////   {
////		// Xa = x
////		// set up X: 700ms for N=2^14, M=3/4 N, R=16
////		MatrixXcf X(N - M, M);
////		for (int i = 0; i < N - M; i++)
////		{
////			X.row(i) = signal.row(0).segment(i + 1, (int)M);
////		}
////
////		//set up x:
////		VectorXcf x(N - M);
////		x = signal.row(0).segment(0, N - M);
////
////		// Partial SVD (see http://arxiv.org/pdf/0909.4061v2.pdf alg 5.1)
////		MatrixXcf Q = RRFcf(X, R); // 600 ms for N=2^14, M=3/4 N, R=16
////		MatrixXcf B = Q.adjoint() * X; // 600 ms for N=2^14, M=3/4 N, R=16
////		VectorXf s(R);
////		MatrixXcf U(R, R);
////		MatrixXcf Vt(R, M);
////		VectorXf superb(M);
////		int result = LAPACKE_cgesvd(LAPACK_COL_MAJOR, 'S', 'S', R, M, B.data(), (int)B.rows(), s.data(), U.data(), (int)U.rows(), Vt.data(), (int)Vt.rows(), superb.data()); // 100 ms for N=2^14, M=3/4 N, R=16
////		if (result != 0)
////      {
////			fmt.str("");
////			fmt << "ERROR: LAPACK CGESVD returned " << result;
////			ErrorMessage((char *)fmt.str().c_str());
////			return(ERR);
////		}
////		//float Q_err = (X - Q * (Q.adjoint() * X)).norm()/X.norm();
////		//fmt.str("");
////		//fmt << "Info: Q_err: " << Q_err << std::endl;
////		//TextMessage((char *)fmt.str().c_str());
////
////		// invert singular values
////		VectorXcf s_Inv(R);
////		for (int i = 0; i < R; i++)
////		{
////			s_Inv(i) = 1 / s(i);
////		}
////
////		// build a = V S^-1 U* x
////		VectorXcf a(M);
////		a = Vt.adjoint() * (s_Inv.asDiagonal() * (U.adjoint() * (Q.adjoint() * x)));
////
////		// do the prediction
////		for (int i = padBack - 1; i >= 0; i--)
////		{
////			new_signal(0, i) = a.conjugate().dot(new_signal.matrix().row(0).segment(i + 1, M));
////		}
////	}
////
////	//pad forward
////	if (padForward > 0)
////   {
////		MatrixXcf X(N - M, M);
////		for (int i = 0; i < N - M; i++)
////		{
////			X.row(i) = signal.row(0).segment(i, (int)M);
////		}
////
////		VectorXcf x(N - M);
////		x = signal.row(0).segment(M, N - M);
////
////		MatrixXcf Q = RRFcf(X, R);
////		MatrixXcf B = Q.adjoint() * X;
////		VectorXf s(R);
////		MatrixXcf U(R, R);
////		MatrixXcf Vt(R, M);
////		VectorXf superb(M);
////		int result = LAPACKE_cgesvd(LAPACK_COL_MAJOR, 'S', 'S', R, M, B.data(), (int)B.rows(), s.data(), U.data(), (int)U.rows(), Vt.data(), (int)Vt.rows(), superb.data());
////		if (result != 0)
////      {
////			fmt.str("");
////			fmt << "ERROR: LAPACK CGESVD returned " << result;
////			ErrorMessage((char *)fmt.str().c_str());
////			return(ERR);
////		}
////
////		VectorXcf s_Inv(R);
////		for (int i = 0; i < R; i++)
////		{
////			s_Inv(i) = 1 / s(i);
////		}
////
////		VectorXcf a(M);
////		a = Vt.adjoint() * (s_Inv.asDiagonal() * (U.adjoint() * (Q.adjoint() * x)));
////
////		for (int i = new_N - padForward; i < new_N; i++)// 600 ms for 2^14
////		{
////			new_signal(0, i) = a.conjugate().dot(new_signal.matrix().row(0).segment(i - M, M));
////		}
////	}
////
////	// Allocate space for the output matrix
////	complex** arrayOut = Eigen2ProspaXXcf(new_signal, new_N, 1);
////
////	par->retVar[1].MakeAndLoadCMatrix2D(arrayOut, new_N, 1);
////	FreeCMatrix2D(arrayOut);
////
////	par->nrRetVar = 1;
////
////	return(OK);
////}
////
////inline void gaussianMXcf(MatrixXcf& M)
////{
////	for (int i = 0; i < M.rows(); ++i)
////   {
////		for (int j = 0; j < M.cols(); ++j)
////      {
////			M(i, j) = { gaussian(gen), gaussian(gen) };
////		}
////	}
////}
////
////// http://arxiv.org/pdf/0909.4061v2.pdf Alg 4.1
////
////MatrixXcf RRFcf(MatrixXcf& A, int l)
////{
////	int m = A.rows(), n = A.cols();
////	MatrixXcf O(n, l);
////	gaussianMXcf(O);
////	MatrixXcf Y = A * O;
////	VectorXcf TAU(l);
////	LAPACKE_cgeqrf(LAPACK_COL_MAJOR, m, l, Y.data(), (int)Y.rows(), TAU.data()); // QR decomp
////	LAPACKE_cungqr(LAPACK_COL_MAJOR, m, l, l, Y.data(), (int)Y.rows(), TAU.data()); // build Q from reflectors
////
////	//see alg 4.4, this helps weight toward larger singular values
////	MatrixXcf Yt = A.adjoint() * Y;
////	LAPACKE_cgeqrf(LAPACK_COL_MAJOR, n, l, Yt.data(), (int)Yt.rows(), TAU.data());
////	LAPACKE_cungqr(LAPACK_COL_MAJOR, n, l, l, Yt.data(), (int)Yt.rows(), TAU.data());
////	Y = A * Yt;
////	LAPACKE_cgeqrf(LAPACK_COL_MAJOR, m, l, Y.data(), (int)Y.rows(), TAU.data());
////	LAPACKE_cungqr(LAPACK_COL_MAJOR, m, l, l, Y.data(), (int)Y.rows(), TAU.data());
////
////	return Y;
////}
//
////short LinearPredict(DLLParameters* par, char *args)
////{
////	short nrArgs;
////	Variable FIDVar;
////	long N, M, R, padBack, padForward;
////
////	nrArgs = ArgScan(par->itfc, args, 5, "FID,numCoeff,maxRank,padBack,padForward", "eeeee", "vllll", &FIDVar, &M, &R, &padBack, &padForward);
////	if (nrArgs < 0)
////		return(nrArgs);
////
////	if (FIDVar.GetType() != CMATRIX2D) {
////		ErrorMessage("First argument to 'linearpredict' should be a complex matrix");
////		return(ERR);
////	}
////
////	if (padBack < 0 || padForward < 0) {
////		ErrorMessage("padBack and padForward must be positive or zero");
////		return(ERR);
////	}
////
////	N = FIDVar.GetDimX();
////	long rows = VarRowSize(&FIDVar);
////	long cols = VarColSize(&FIDVar);
////
////	complex** arrayIn = FIDVar.GetCMatrix2D();
////
////	ArrayXXcf signal = Prospa2EigenXXcf(arrayIn, N, 1);
////
////	//use maximum rank if it is 0 or too big
////	if (R <= 0 || R>MIN(N, N - M)) {
////		R = MIN(N, N - M);
////		fmt.str("");
////		fmt << "Warning: Rank set to " << R << std::endl;
////		TextMessage((char *)fmt.str().c_str());
////	}
////
////	// Xa = x
////	// set up X:
////	MatrixXcf X_F(N - M, M);
////	MatrixXcf X_B(N - M, M);
////	for (int i = 0; i < N - M; i++)
////	{
////		X_F.row(i) = signal.row(0).segment(i, (int)M);
////		X_B.row(i) = signal.row(0).segment(i + 1, (int)M);
////	}
////
////	//set up x:
////	VectorXcf x_F(N - M);
////	VectorXcf x_B(N - M);
////	x_F = signal.row(0).segment(M, N - M);
////	x_B = signal.row(0).segment(0, N - M);
////
////	// LAPACK SVD
////	int result;
////	VectorXf s_F(MIN(N - M, M));
////	MatrixXcf U_F(N - M, MIN(N - M, M));
////	MatrixXcf Vt_F(MIN(N - M, M), M);
////	VectorXf superb(N);
////	result = LAPACKE_cgesvd(LAPACK_COL_MAJOR, 'S', 'S', N - M, M, X_F.data(), (int)X_F.rows(), s_F.data(), U_F.data(), (int)U_F.rows(), Vt_F.data(), (int)Vt_F.rows(), superb.data());
////	if (result != 0) {
////		fmt.str("");
////		fmt << "ERROR: LAPACK CGESVD returned " << result;
////		ErrorMessage((char *)fmt.str().c_str());
////		return(ERR);
////	}
////	//fmt.str("");
////	//fmt << "s Forward: " << std::endl << s_F.head(R).transpose() << std::endl;
////	//TextMessage((char *)fmt.str().c_str());
////	VectorXf s_B(MIN(N - M, M));
////	MatrixXcf U_B(N - M, MIN(N - M, M));
////	MatrixXcf Vt_B(MIN(N - M, M), M);
////	result = LAPACKE_cgesvd(LAPACK_COL_MAJOR, 'S', 'S', N - M, M, X_B.data(), (int)X_B.rows(), s_B.data(), U_B.data(), (int)U_B.rows(), Vt_B.data(), (int)Vt_B.rows(), superb.data());
////	if (result != 0) {
////		fmt.str("");
////		fmt << "ERROR: LAPACK CGESVD returned " << result;
////		ErrorMessage((char *)fmt.str().c_str());
////		return(ERR);
////	}
////	//fmt.str("");
////	//fmt << "s Back: " << std::endl << s_B.head(R).transpose() << std::endl;
////	//TextMessage((char *)fmt.str().c_str());
////
////	// build X^-1
////	VectorXcf s_Inv_F(M);
////	VectorXcf s_Inv_B(M);
////	for (int i = 0; i < M; i++)
////	{
////		if (i<R) {
////			s_Inv_F(i) = 1 / s_F(i);
////			s_Inv_B(i) = 1 / s_B(i);
////		}
////		else {
////			s_Inv_F(i) = 0;
////			s_Inv_B(i) = 0;
////		}
////	}
////
////	// build a = V S^-1 U* x
////	VectorXcf a_F(M);
////	VectorXcf a_B(M);
////
////	a_F = Vt_F.adjoint() * s_Inv_F.asDiagonal() * U_F.adjoint() * x_F;
////	a_B = Vt_B.adjoint() * s_Inv_B.asDiagonal() * U_B.adjoint() * x_B;
////
////	long new_N = N + padBack + padForward;
////
////	ArrayXXcf new_signal(1, new_N);
////	new_signal.block(0, padBack, 1, N) = signal;
////
////	for (int i = padBack - 1; i >= 0; i--)
////	{
////		new_signal(0, i) = a_B.conjugate().dot(new_signal.matrix().row(0).segment(i + 1, M));
////	}
////
////	for (int i = new_N - padForward; i < new_N; i++)
////	{
////		new_signal(0, i) = a_F.conjugate().dot(new_signal.matrix().row(0).segment(i - M, M));
////	}
////
////
////	//VectorXcf s_B(M);
////	////VectorXcf s_Inv_B_F(SVD_B_F.singularValues().size());
////	//VectorXcf s_B_B(SVD_B_B.singularValues().size());
////	//for (int i = 0; i < M; i++)
////	//{
////	//	s_B(i) = SVD_B.singularValues()(i);
////	//}
////	//for (int i = 0; i < SVD_B_B.singularValues().size(); i++)
////	//{
////	//	s_B_B(i) = SVD_B_B.singularValues()(i);
////	//}
////
////	//new_signal.row(0).segment(2 * R, R) = s_B.head(R);
////	//new_signal.row(0).segment(3 * R, s_B_B.size()) = s_B_B;
////	//new_signal.row(0).head(M) = a_F;
////
////	// Allocate space for the output matrix
////	complex** arrayOut = Eigen2ProspaXXcf(new_signal, new_N, 1);
////
////	par->retVar[1].MakeAndLoadCMatrix2D(arrayOut, new_N, 1);
////	FreeCMatrix2D(arrayOut);
////
////	par->nrRetVar = 1;
////
////	return(OK);
////}
//
////MatrixXcf ARRFcf(MatrixXcf A, float eps, int r, int max_rank) {// http://arxiv.org/pdf/0909.4061v2.pdf Alg 4.2
////	//eps: 1e-3
////	//r: 10
////	int m = A.rows(), n = A.cols();
////	//eps = eps / (10.0*std::sqrt(2.0 / std::_Pi));
////	//A = A / A.colwise().norm().minCoeff();
////	MatrixXcf I = MatrixXcf::Identity(m, m);
////	MatrixXcf w(n, 1);
////	MatrixXcf W(n, r);
////	gaussianMXcf(W); // fill with complex gaussian samples
////	MatrixXcf Y = A * W;
////	MatrixXcf Q(m, 1);
////	int j = 0;
////	float err = Y.rightCols(r).colwise().norm().maxCoeff();
////	float lasterr = 2 * err;
////	float err_ratio = lasterr / err - 1.0f;
////	while (err_ratio > eps && j<max_rank) {
////		if (j > 0) {
////			Y.col(j).applyOnTheLeft(I - Q * Q.adjoint());
////			Q.conservativeResize(NoChange, Q.cols() + 1);
////		}
////		Q.col(j) = Y.col(j).normalized();
////
////		W.conservativeResize(NoChange, W.cols() + 1);
////		gaussianMXcf(w);
////		W.rightCols(1) = w;
////
////		Y.conservativeResize(NoChange, Y.cols() + 1);
////		Y.col(j + r) = (I - Q * Q.adjoint()) * A * W.col(j + r);
////
////		for (int i = j + 1; i < j + r; i++) {
////			Y.col(i) = Y.col(i) - Q.col(j) * Q.col(j).dot(Y.col(i));
////		}
////
////		j++;
////		lasterr = err;
////		err = Y.rightCols(r).colwise().norm().maxCoeff();
////		err_ratio = std::abs(lasterr / err - 1.0f);
////	}
////	return Q;
////}