#include "pch.h"
#include "CppUnitTest.h"
#include "../ImportFunctions.h"
#include "../UtilityFunctions.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests
{
	TEST_CLASS(UnitTests)
	{
	public:
		
		TEST_METHOD(TransSVDTest)
		{
			auto testPoints = std::vector<coord>{ coord(0, 0), coord(1, 1) };

			//auto results = getTransSVD(std::vector<coord>({ coord(0,0), coord(1,1) }), std::vector<coord>({ coord(0, 0), coord(1, 1) }));
			Assert::AreEqual(testPoints, std::vector<coord>{ coord(0, 0), coord(1, 1) });
			
			
		}
	};
}
