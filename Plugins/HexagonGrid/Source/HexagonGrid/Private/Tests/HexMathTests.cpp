#include "HexMath.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(HexDirectionToAxialNeighbourIndex, "Private.Tests.HexDirectionToAxialNeighbourIndex",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool HexDirectionToAxialNeighbourIndex::RunTest(const FString& Parameters)
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(HexAxialToOffset, "Private.Tests.HexAxialToOffset",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool HexAxialToOffset::RunTest(const FString& Parameters)
{
	using namespace HexMath;

	TestEqual(TEXT("0:0 == 0:0"), HexMathAxial::AxialToOffset<EHexOffsetLayout::FlatTopOddQ>(FAxialCoord(0, 0)), FOffsetCoord(0, 0));
	TestEqual(TEXT("0:-1 == 0:-1"), HexMathAxial::AxialToOffset<EHexOffsetLayout::FlatTopOddQ>(FAxialCoord(0, -1)), FOffsetCoord(0, -1));
	TestEqual(TEXT("-1:0 == -1:-1"), HexMathAxial::AxialToOffset<EHexOffsetLayout::FlatTopOddQ>(FAxialCoord(-1, 0)), FOffsetCoord(-1, -1));
	TestEqual(TEXT("-1:1 == -1:0"), HexMathAxial::AxialToOffset<EHexOffsetLayout::FlatTopOddQ>(FAxialCoord(-1, 1)), FOffsetCoord(-1, 0));
	TestEqual(TEXT("0:1 == 0:1"), HexMathAxial::AxialToOffset<EHexOffsetLayout::FlatTopOddQ>(FAxialCoord(0, 1)), FOffsetCoord(0, 1));
	TestEqual(TEXT("1:0 == 1:0"), HexMathAxial::AxialToOffset<EHexOffsetLayout::FlatTopOddQ>(FAxialCoord(1, 0)), FOffsetCoord(1, 0));
	TestEqual(TEXT("1:-1 == 1:-1"), HexMathAxial::AxialToOffset<EHexOffsetLayout::FlatTopOddQ>(FAxialCoord(1, -1)), FOffsetCoord(1, -1));
	TestEqual(TEXT("-2:-1 == -2:-2"), HexMathAxial::AxialToOffset<EHexOffsetLayout::FlatTopOddQ>(FAxialCoord(-2, -1)), FOffsetCoord(-2, -2));
	TestEqual(TEXT("1:2 == 1:2"), HexMathAxial::AxialToOffset<EHexOffsetLayout::FlatTopOddQ>(FAxialCoord(1, 2)), FOffsetCoord(1, 2));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(HexOffsetToAxial, "Private.Tests.HexOffsetToAxial",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool HexOffsetToAxial::RunTest(const FString& Parameters)
{
	using namespace HexMath;

	TestEqual(TEXT("0:0 == 0:0"), HexMathAxial::OffsetToAxial<EHexOffsetLayout::FlatTopOddQ>(FOffsetCoord(0, 0)), FAxialCoord(0, 0));
	TestEqual(TEXT("0:-1 == 0:-1"), HexMathAxial::OffsetToAxial<EHexOffsetLayout::FlatTopOddQ>(FOffsetCoord(0, -1)), FAxialCoord(0, -1));
	TestEqual(TEXT("-1:-1 == -1:0"), HexMathAxial::OffsetToAxial<EHexOffsetLayout::FlatTopOddQ>(FOffsetCoord(-1, -1)), FAxialCoord(-1, 0));
	TestEqual(TEXT("-1:0 == -1:1"), HexMathAxial::OffsetToAxial<EHexOffsetLayout::FlatTopOddQ>(FOffsetCoord(-1, 0)), FAxialCoord(-1, 1));
	TestEqual(TEXT("0:1 == 0:1"), HexMathAxial::OffsetToAxial<EHexOffsetLayout::FlatTopOddQ>(FOffsetCoord(0, 1)), FAxialCoord(0, 1));
	TestEqual(TEXT("1:0 == 1:0"), HexMathAxial::OffsetToAxial<EHexOffsetLayout::FlatTopOddQ>(FOffsetCoord(1, 0)), FAxialCoord(1, 0));
	TestEqual(TEXT("1:-1 == 1:-1"), HexMathAxial::OffsetToAxial<EHexOffsetLayout::FlatTopOddQ>(FOffsetCoord(1, -1)), FAxialCoord(1, -1));
	TestEqual(TEXT("-2:-2 == -2:-1"), HexMathAxial::OffsetToAxial<EHexOffsetLayout::FlatTopOddQ>(FOffsetCoord(-2, -2)), FAxialCoord(-2, -1));
	TestEqual(TEXT("1:2 == 1:2"), HexMathAxial::OffsetToAxial<EHexOffsetLayout::FlatTopOddQ>(FOffsetCoord(1, 2)), FAxialCoord(1, 2));
	return true;
}
