#include "pch.h"
#include "../st-visualizer/JSONParser.h"
#include "CppUnitTest.h"
#include "../st-visualizer/ImportFunctions.h"
#include <math.h>;
#include <fstream>;
#include <sstream>;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace ImportTests
{
	TEST_CLASS(SVDTests)
	{
	public:
		TEST_METHOD(getSVDRotationTest)
		{
			//The main purpose to this set of tests is to make sure this works with cases of translation, and rotation

			//Row coordinate matrix
			typedef Eigen::Matrix<float, Eigen::Dynamic, 2> rowMat;
			typedef Eigen::Matrix<float, 2, Eigen::Dynamic> colMat;

			auto i = 0;
			//Generic Rotation Testing Function
			auto genRotTest = [&i](rowMat source, rowMat target) {
				i++;
				// Send centroids to zero
				auto a = source.transpose();
				auto b = target.transpose();

				//Test their rotations
				auto rotation = getSVDRotation(a, b);
				std::stringstream failStream;
				failStream << i << std::endl<< a << std::endl << std::endl << b << std::endl << std::endl << rotation << std::endl << std::endl << rotation * a;;
				std::string failString = failStream.str();
				std::wstring widestr = std::wstring(failString.begin(), failString.end());
				const wchar_t* widecstr = widestr.c_str();
				Assert::IsTrue(translateToZeroCentroid(rotation * a).isApprox(translateToZeroCentroid(b)), widecstr);
			};

			int maxI = 1000;
			float pi = 3.1415926535;

			//Number of different matrixes to try
			for (int j = 0; j < 5; j++) {
				//Try 100 random tests with large length
				colMat m = colMat::Random(2, 20) * 100;
				for (int i = 0; i < maxI; i++) {
					auto rotation = Eigen::Rotation2D<float>(2 * pi * i / maxI).toRotationMatrix();
					auto translatoin = Eigen::Translation2f(Eigen::Vector2f::Random() * 100);
					Eigen::Transform<float, 2, Eigen::Affine> trans = translatoin * rotation;
					

					genRotTest(
						m.transpose(),
						//Test random values against all radians possible within range
						(trans * m).transpose()
					);
				}
			}
		}

		TEST_METHOD(getTransSVDTest) {
			//The main purpose to this set of tests is to make sure this works with cases of scaling, translation, and rotation

			//Row coordinate matrix
			//Row coordinate matrix
			typedef Eigen::Matrix<float, Eigen::Dynamic, 2> rowMat;
			typedef Eigen::Matrix<float, 2, Eigen::Dynamic> colMat;

			auto i = 0;


			int maxI = 1000;
			float pi = 3.14159265358979323846264338327950288419716939937510f;

			auto genRotTest = [&i](std::vector<coord> source, std::vector<coord> target, std::vector<coord> finalSource, std::vector<coord> finalTest) {
				i++;

				auto func = getTransSVD(source, target);
				auto a = vectorToMatrix(func(finalSource));


				std::stringstream failStream;
				failStream << i << std::endl << a << std::endl << std::endl << vectorToMatrix(finalTest) << std::endl << std::endl << vectorToMatrix(source) << std::endl << std::endl << vectorToMatrix(target);

				std::string failString = failStream.str();
				std::wstring widestr = std::wstring(failString.begin(), failString.end());
				const wchar_t* widecstr = widestr.c_str();

				Assert::IsTrue(a.isApprox(vectorToMatrix(finalTest)), widecstr);

			};

			//Number of different matrixes to try
			for (int j = 0; j < 5; j++) {
				//Try 100 random tests with large length and a centroid of zero
				colMat m = colMat::Random(2, 10) * 100;
				colMat m2 = colMat::Random(2, 30) * 100;
				for (int i = 200; i < maxI; i++) {
					auto rotation = Eigen::Rotation2D<float>(2 * pi * i / maxI);
					auto translatoin = Eigen::Translation2f(Eigen::Vector2f::Random() * 100);
					Eigen::Transform<float, 2, Eigen::Affine> trans = translatoin * rotation;
					genRotTest(
						matrixToVector(m),
						matrixToVector(trans * m),
						matrixToVector(m2),
						matrixToVector(trans * m2)
					);
				}
			}
		}

		TEST_METHOD(SVDMathematica) {
			using json = nlohmann::json;
			int i = 0;
			auto genRotTest = [&i](std::vector<coord> source, std::vector<coord> target, std::vector<coord> finalSource, std::vector<coord> finalTest) {
				i++;

				auto func = getTransSVD(source, target);
				auto a = vectorToMatrix(func(finalSource));


				std::stringstream failStream;
				failStream << i << std::endl << a << std::endl << std::endl << vectorToMatrix(finalTest) << std::endl << std::endl << vectorToMatrix(source) << std::endl << std::endl << vectorToMatrix(target);

				std::string failString = failStream.str();
				std::wstring widestr = std::wstring(failString.begin(), failString.end());
				const wchar_t* widecstr = widestr.c_str();

				/*if (!a.isApprox(vectorToMatrix(finalTest))) {
					std::cout << "FIRST" << std::endl << a - vectorToMatrix(finalTest) << std::endl << std::endl;
					std::cout << "Result" << std::endl << vectorToMatrix(finalSource).rowwise().mean() << std::endl << std::endl;
					std::cout << "Test" << std::endl << vectorToMatrix(finalTest).rowwise().mean() << std::endl << std::endl;

				}*/
				Assert::IsTrue(a.isApprox(vectorToMatrix(finalTest)));

			};
			auto jsonToMatrix = [](const json& source) {
				//This should only be used for testing. Not logic safe.

				Eigen::Matrix2Xf a(2, source.size());

				for (int i = 0; i < source.size(); i++) {
					a(0, i) = source[i][0];
					a(1, i) = source[i][1];
				}
				return a;
			};
			// read a JSON file
			std::ifstream file("C:\\Users\\Aiden McIlraith\\Documents\\GitHub\\st-visualizer\\UnitTest\\svd.json");
			auto a = file.is_open();
			json j = json::parse(file);

			for (json& test : j) {
				auto alignSource = jsonToMatrix(test[0]);
				auto alignTarget = jsonToMatrix(test[1]);
				auto mapSource = jsonToMatrix(test[2]);
				auto mapTarget = jsonToMatrix(test[3]);
				genRotTest(
					matrixToVector(alignSource),
					matrixToVector(alignTarget),
					matrixToVector(mapSource),
					matrixToVector(mapTarget)
				);
			}

		}

	};
}
