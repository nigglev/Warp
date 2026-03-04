#include "HexMath.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(HexMathTests, "Private.Tests.HexMathTests",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool HexMathTests::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("0 == 0"), HexMath::HexMathAxial::DirectionToAxialNeighbourIndex(0), 0);
	TestEqual(TEXT("1 == 1"), HexMath::HexMathAxial::DirectionToAxialNeighbourIndex(1), 1);
	TestEqual(TEXT("-1 == 5"), HexMath::HexMathAxial::DirectionToAxialNeighbourIndex(-1), 5);
	TestEqual(TEXT("6 == 0"), HexMath::HexMathAxial::DirectionToAxialNeighbourIndex(6), 0);
	TestEqual(TEXT("-6 == 0"), HexMath::HexMathAxial::DirectionToAxialNeighbourIndex(-6), 0);
	TestEqual(TEXT("7 == 1"), HexMath::HexMathAxial::DirectionToAxialNeighbourIndex(7), 1);
	TestEqual(TEXT("-7 == 5"), HexMath::HexMathAxial::DirectionToAxialNeighbourIndex(-7), 5);
	TestEqual(TEXT("-13 == 5"), HexMath::HexMathAxial::DirectionToAxialNeighbourIndex(-13), 5);
	return true;
}
