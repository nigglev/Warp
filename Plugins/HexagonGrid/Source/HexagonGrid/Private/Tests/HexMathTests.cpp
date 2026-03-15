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

	TestEqual(TEXT("0:0 == 0:0"), HexMath::HexMathAxial::AxialToOffset<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FAxialCoord(0, 0)), HexMath::FOffsetCoord(0, 0));
	TestEqual(TEXT("0:-1 == 0:-1"), HexMath::HexMathAxial::AxialToOffset<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FAxialCoord(0, -1)), HexMath::FOffsetCoord(0, -1));
	TestEqual(TEXT("-1:0 == -1:-1"), HexMath::HexMathAxial::AxialToOffset<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FAxialCoord(-1, 0)), HexMath::FOffsetCoord(-1, -1));
	TestEqual(TEXT("-1:1 == -1:0"), HexMath::HexMathAxial::AxialToOffset<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FAxialCoord(-1, 1)), HexMath::FOffsetCoord(-1, 0));
	TestEqual(TEXT("0:1 == 0:1"), HexMath::HexMathAxial::AxialToOffset<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FAxialCoord(0, 1)), HexMath::FOffsetCoord(0, 1));
	TestEqual(TEXT("1:0 == 1:0"), HexMath::HexMathAxial::AxialToOffset<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FAxialCoord(1, 0)), HexMath::FOffsetCoord(1, 0));
	TestEqual(TEXT("1:-1 == 1:-1"), HexMath::HexMathAxial::AxialToOffset<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FAxialCoord(1, -1)), HexMath::FOffsetCoord(1, -1));
	TestEqual(TEXT("-2:-1 == -2:-2"), HexMath::HexMathAxial::AxialToOffset<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FAxialCoord(-2, -1)), HexMath::FOffsetCoord(-2, -2));
	TestEqual(TEXT("1:2 == 1:2"), HexMath::HexMathAxial::AxialToOffset<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FAxialCoord(1, 2)), HexMath::FOffsetCoord(1, 2));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(HexOffsetToAxial, "Private.Tests.HexOffsetToAxial",
								 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool HexOffsetToAxial::RunTest(const FString& Parameters)
{

	TestEqual(TEXT("0:0 == 0:0"), HexMath::HexMathAxial::OffsetToAxial<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FOffsetCoord(0, 0)), HexMath::FAxialCoord(0, 0));
	TestEqual(TEXT("0:-1 == 0:-1"), HexMath::HexMathAxial::OffsetToAxial<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FOffsetCoord(0, -1)), HexMath::FAxialCoord(0, -1));
	TestEqual(TEXT("-1:-1 == -1:0"), HexMath::HexMathAxial::OffsetToAxial<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FOffsetCoord(-1, -1)), HexMath::FAxialCoord(-1, 0));
	TestEqual(TEXT("-1:0 == -1:1"), HexMath::HexMathAxial::OffsetToAxial<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FOffsetCoord(-1, 0)), HexMath::FAxialCoord(-1, 1));
	TestEqual(TEXT("0:1 == 0:1"), HexMath::HexMathAxial::OffsetToAxial<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FOffsetCoord(0, 1)), HexMath::FAxialCoord(0, 1));
	TestEqual(TEXT("1:0 == 1:0"), HexMath::HexMathAxial::OffsetToAxial<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FOffsetCoord(1, 0)), HexMath::FAxialCoord(1, 0));
	TestEqual(TEXT("1:-1 == 1:-1"), HexMath::HexMathAxial::OffsetToAxial<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FOffsetCoord(1, -1)), HexMath::FAxialCoord(1, -1));
	TestEqual(TEXT("-2:-2 == -2:-1"), HexMath::HexMathAxial::OffsetToAxial<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FOffsetCoord(-2, -2)), HexMath::FAxialCoord(-2, -1));
	TestEqual(TEXT("1:2 == 1:2"), HexMath::HexMathAxial::OffsetToAxial<HexMath::EHexOffsetLayout::FlatTopOddQ>(HexMath::FOffsetCoord(1, 2)), HexMath::FAxialCoord(1, 2));
	return true;
}
