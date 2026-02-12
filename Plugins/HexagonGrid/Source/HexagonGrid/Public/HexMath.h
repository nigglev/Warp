#pragma once
#include "CoreMinimal.h"

namespace HexMath
{
	using HexReal = float;
	using HexInt = int32;
	
	static_assert(std::is_signed_v<HexInt>, "HexInt must be signed");
	
	static constexpr bool bIsInt64 = std::is_same_v<HexInt, int64>;
	
	constexpr HexReal sqrt3 = 1.7320508075688772935274463415059;
	constexpr HexReal sqrt3_2 = sqrt3 / 2.;
	constexpr HexReal sqrt3_3 = sqrt3 / 3.;	
	
	//https://www.redblobgames.com/grids/hexagons/
	
	enum class EHexOffsetLayout : uint8
	{
		FlatTopOddQ = 0,
		FlatTopEvenQ,
		PointyTopOddR,
		PointyTopEvenR
	};
	
	template<EHexOffsetLayout OffsetType>
	static constexpr bool bIsFlat =	OffsetType == EHexOffsetLayout::FlatTopOddQ || OffsetType == EHexOffsetLayout::FlatTopEvenQ;
	
	template<EHexOffsetLayout OffsetType>
	static constexpr bool bIsOdd =	OffsetType == EHexOffsetLayout::FlatTopOddQ || OffsetType == EHexOffsetLayout::PointyTopOddR;
	
#pragma region OffsetCoord
	struct FOffsetCoord
	{
		HexInt Up = INT32_MAX;
		HexInt Right = INT32_MAX;
		
		FOffsetCoord() = default;
		FOffsetCoord(HexInt InRight, HexInt InUp) : Up(InUp), Right(InRight) {}
		
		friend auto operator<=>(const FOffsetCoord&, const FOffsetCoord&) = default;
		
		FString ToString() const
		{
			return FString::Printf(TEXT("(%d, %d)"), Right, Up);
		}
	};
	
	inline FOffsetCoord operator+(const FOffsetCoord& LHS, const FOffsetCoord& RHS) { return FOffsetCoord(LHS.Right + RHS.Right, LHS.Up + RHS.Up); }
	inline FOffsetCoord operator-(const FOffsetCoord& LHS, const FOffsetCoord& RHS) { return FOffsetCoord(LHS.Right - RHS.Right, LHS.Up - RHS.Up); }
	
	template<typename T>
	FOffsetCoord operator*(T LHS, const FOffsetCoord& RHS) { return FOffsetCoord(LHS * RHS.Right, LHS * RHS.Up); }
	template<typename T>
	FOffsetCoord operator*(const FOffsetCoord& LHS, T RHS) { return FOffsetCoord(LHS.Right * RHS, LHS.Up * RHS); }
	
	struct FOffsetRealCoord
	{
		HexReal Up = 0;
		HexReal Right = 0;
		
		FOffsetRealCoord() = default;
		FOffsetRealCoord(HexReal InRight, HexReal InUp) : Up(InUp), Right(InRight) {}
		
		FString ToString() const { return FString::Printf(TEXT("(%.3f, %.3f)"), Right, Up); }
	};
	
	inline FOffsetRealCoord operator+(const FOffsetRealCoord& LHS, const FOffsetRealCoord& RHS) { return FOffsetRealCoord(LHS.Right + RHS.Right, LHS.Up + RHS.Up); }
	
	template<typename T>
	FOffsetRealCoord operator*(T LHS, const FOffsetRealCoord& RHS) { return FOffsetRealCoord(LHS * RHS.Right, LHS * RHS.Up); }
	template<typename T>
	FOffsetRealCoord operator*(const FOffsetRealCoord& LHS, T RHS) { return FOffsetRealCoord(LHS.Right * RHS, LHS.Up * RHS); }
	
#pragma endregion
	
	struct FAxialRealCoord
	{
		HexReal Q = 0; //Col vert for flat-top
		HexReal R = 0; //120 degree rows
		
		FAxialRealCoord() = default;
		FAxialRealCoord(HexReal InQ, HexReal InR) : Q(InQ), R(InR) {}
		
		FString ToString() const { return FString::Printf(TEXT("(%.3f, %.3f)"), Q, R); }
	};
	
	struct FAxialCoord
	{
		HexInt Q = INT32_MAX; //Col vert for flat-top
		HexInt R = INT32_MAX; //120 degree rows
		
		FAxialCoord() = default;
		FAxialCoord(HexInt InQ, HexInt InR) : Q(InQ), R(InR) {}
		
		FString ToString() const { return FString::Printf(TEXT("(%d, %d)"), Q, R); }
	};
	
	inline FAxialCoord operator+(const FAxialCoord& LHS, const FAxialCoord& RHS) { return FAxialCoord(LHS.Q + RHS.Q, LHS.R + RHS.R); }
	inline FAxialCoord operator-(const FAxialCoord& LHS, const FAxialCoord& RHS) { return FAxialCoord(LHS.Q - RHS.Q, LHS.R - RHS.R); }
	
	inline bool operator==(const FAxialCoord& LHS, const FAxialCoord& RHS) { return LHS.Q == RHS.Q && LHS.R == RHS.R; }
	inline bool operator!=(const FAxialCoord& LHS, const FAxialCoord& RHS) { return LHS.Q != RHS.Q || LHS.R != RHS.R; }
	
	FORCEINLINE uint32 GetTypeHash(const FAxialCoord& Key)
	{
		uint32 Hash = 0;
		Hash = HashCombine(Hash, ::GetTypeHash(Key.Q));
		Hash = HashCombine(Hash, ::GetTypeHash(Key.R));
		return Hash;
	}
	
	inline HexInt AxialDistance(const FAxialCoord& LHS, const FAxialCoord& RHS)
	{
		const HexInt dq = LHS.Q - RHS.Q;
		const HexInt dr = LHS.R - RHS.R;
		return (FMath::Abs(dq) + FMath::Abs(dr) + FMath::Abs(dq + dr)) / 2;
	}

	namespace HexMathAxial
	{
		//https://www.redblobgames.com/grids/hexagons/
		
		template<EHexOffsetLayout OffsetType>
		FAxialRealCoord OffsetToRealAxial(const FOffsetRealCoord& InWorldPoint, float InHexSize)
		{
			HexReal q; 
			HexReal r;
			
			if constexpr (bIsFlat<OffsetType>)
			{
				q = 2.f / 3.f * InWorldPoint.Right / InHexSize;
				r = (sqrt3_3 * InWorldPoint.Up - InWorldPoint.Right / 3.f) / InHexSize;
			}
			else
			{
				q = (sqrt3_3 * InWorldPoint.Right - InWorldPoint.Up / 3) / InHexSize;
				r = 2.f / 3.f * InWorldPoint.Up / InHexSize;
			}
	
			return FAxialRealCoord(q, r);
		}
		
		inline FAxialCoord CubeRoundAxial(const FAxialRealCoord& InCoord)
		{
			HexReal y = -InCoord.Q - InCoord.R;
	
			int32 rq = FMath::RoundToInt(InCoord.Q);
			int32 ry = FMath::RoundToInt(y);
			int32 rr = FMath::RoundToInt(InCoord.R);
	
			const HexReal dq = FMath::Abs(rq - InCoord.Q);
			const HexReal dy = FMath::Abs(ry - y);
			const HexReal dr = FMath::Abs(rr - InCoord.R);
	
			if (dq > dy && dq > dr)      rq = -ry - rr;
			else if (dy > dr)            ry = -rq - rr;
			else                         rr = -rq - ry;
	
			return FAxialCoord(rq, rr); // (q,r)
		}
		
		template<EHexOffsetLayout OffsetType>
		FAxialCoord OffsetToAxial(const FOffsetRealCoord& InOffsetCoord, float InHexSize)
		{
			FAxialRealCoord A = OffsetToRealAxial<OffsetType>(InOffsetCoord, InHexSize);
			const FAxialCoord Axial = CubeRoundAxial(A);  // (q,r) int
			return Axial;
		}
		
		template<EHexOffsetLayout OffsetType>
		FOffsetCoord AxialToOffset(const FAxialCoord& InACoord)
		{
			const HexInt Q1 = InACoord.Q & 1;
			const HexInt R1 = InACoord.R & 1;
			if constexpr (OffsetType == EHexOffsetLayout::FlatTopOddQ)
			{
				const HexInt row = InACoord.R + (InACoord.Q - Q1) / 2;
				return FOffsetCoord(InACoord.Q, row);
			}
			else if constexpr (OffsetType == EHexOffsetLayout::FlatTopEvenQ)
			{
				const HexInt row = InACoord.R + (InACoord.Q + Q1) / 2;
				return FOffsetCoord(InACoord.Q, row);
			}
			else if constexpr (OffsetType == EHexOffsetLayout::PointyTopOddR)
			{
				const HexInt col = InACoord.Q + (InACoord.R - R1) / 2;
				return FOffsetCoord(col, InACoord.R);
			}
			else // PointyTopEvenR
			{
				const HexInt col = InACoord.Q + (InACoord.R + R1) / 2;
				return FOffsetCoord(col, InACoord.R);
			}
		}
		
		template<EHexOffsetLayout OffsetType>
		FOffsetCoord WorldToOffset(const FOffsetRealCoord& P, float R)
		{
			FAxialRealCoord A = OffsetToRealAxial<OffsetType>(P, R);
			const FAxialCoord Axial = CubeRoundAxial(A);  // (q,r) int
			return AxialToOffset<OffsetType>(Axial);      // (col,row) int
		}
	
		static constexpr uint8 AxialNeighbourCount = 6;
		static const FAxialCoord AxialNeighboursShifts[AxialNeighbourCount] = {
			{+1,  0},
			{+1, -1},
			{ 0, -1},
			{-1,  0},
			{-1, +1},
			{ 0, +1},
		};
		
		inline void IterateAxialNeighbours(const FAxialCoord& InAxialCenter, int32 InHexRadius, 
			const TFunctionRef<void(const FAxialCoord&)>& InHandler)
		{
			for (int32 q = -InHexRadius; q <= InHexRadius; ++q)
			{
				const int32 r1 = FMath::Max(-InHexRadius, -q - InHexRadius);
				const int32 r2 = FMath::Min( InHexRadius, -q + InHexRadius);
	
				for (int32 r = r1; r <= r2; ++r)
				{
					FAxialCoord Cell = InAxialCenter + FAxialCoord(q, r);
					InHandler(Cell);
				}
			}
		}
	
		inline float GetAngle(int32 InSegmentCount) { return 360.f / InSegmentCount; }
	
		inline float GetEdgeLength(float InCircularRadius, int32 InSegmentCount)
		{
			float A = GetAngle(InSegmentCount);
			float SinValue = FMath::Sin(FMath::DegreesToRadians(A) / 2.f);
			return 2 * InCircularRadius * SinValue;
		}
	
		inline float GetInRadius(float InCircularRadius, int32 InSegmentCount)
		{
			float E = GetEdgeLength(InCircularRadius, InSegmentCount);
			return FMath::Sqrt(InCircularRadius * InCircularRadius - E * E / 4);
		}
	
		//Q along UE X, R along UE Y
		// inline FVector AxialToWorld(int32 Q, int32 R, float InCircularRadius, float InZOffset = 0, bool InPointyTop = false)
		// {
		// 	HexReal x;
		// 	HexReal y;
		//
		// 	if (InPointyTop)
		// 	{
		// 		x = InCircularRadius * sqrt3 * (static_cast<float>(Q) + static_cast<float>(R) * 0.5f);
		// 		y = InCircularRadius * 1.5f * static_cast<float>(R);
		// 	}
		// 	else
		// 	{
		// 		x = InCircularRadius * 1.5f * static_cast<float>(Q);
		// 		y = InCircularRadius * sqrt3 * (static_cast<float>(R) + static_cast<float>(Q) * 0.5f);
		// 	}
		//
		// 	return FVector(x, y, InZOffset);
		// }
	
		// static FIntPoint WorldToAxial(const FVector& InWorldPoint, float InHexSize, bool InPointyTop = false)
		// {
		// 	FVector2D V = WorldToAxialFractional(InWorldPoint, InHexSize, InPointyTop);
		// 	return CubeRoundAxial(V.X, V.Y);
		// }
		//
		// static FIntPoint WorldToChunkCoord(const FVector& InWorldPoint, float InHexSize, int32 R, bool InPointyTop = false)
		// {
		// 	const int32 Stride = 1; //2 * R + 1;
		// 	const FVector2D Ax = WorldToAxialFractional(InWorldPoint, InHexSize, InPointyTop);
		// 	return CubeRoundAxial(Ax.X / static_cast<float>(Stride), Ax.Y / static_cast<float>(Stride));
		// }
	
	}

	namespace HexMathOffset
	{
		
	
		constexpr HexReal GetH(float InCircularRadius) { return InCircularRadius * sqrt3; } //flat-top
		constexpr HexReal GetW(float InCircularRadius) { return InCircularRadius * sqrt3; } //pointy-top
	
		template<EHexOffsetLayout OffsetType>
		FOffsetRealCoord ColBasis(float InCircularRadius)
		{
			if constexpr (bIsFlat<OffsetType>) { return FOffsetRealCoord(1.5 * InCircularRadius, 0); }
			else { return FOffsetRealCoord(GetW(InCircularRadius), 0); }
		}

		template<EHexOffsetLayout OffsetType>
		FOffsetRealCoord RowBasis(float InCircularRadius)
		{
			if constexpr (bIsFlat<OffsetType>) { return FOffsetRealCoord(0, GetH(InCircularRadius));}
			else { return FOffsetRealCoord(0, 1.5 * InCircularRadius); }
		}
	
		template<EHexOffsetLayout OffsetType>
		FOffsetRealCoord ParityShift(float InCircularRadius)
		{
			if constexpr (bIsFlat<OffsetType>) { return FOffsetRealCoord(0, 0.5 * GetH(InCircularRadius)); }
			else { return FOffsetRealCoord(0.5 * GetW(InCircularRadius), 0); }
		}
	
		template<EHexOffsetLayout OffsetType>
		uint8 Parity(uint32 InColRow)
		{
			if constexpr (bIsOdd<OffsetType>) {	return static_cast<uint8>(InColRow & 1); }
			else { return static_cast<uint8>((InColRow + 1) & 1); }
		}
	
		template<EHexOffsetLayout OffsetType>
		FOffsetRealCoord GetHexOffsetPos(uint32 InCol, uint32 InRow, float InCircularRadius)
		{
			if constexpr (bIsFlat<OffsetType>)
			{
				return InCol * ColBasis<OffsetType>(InCircularRadius) + InRow * RowBasis<OffsetType>(InCircularRadius) + Parity<OffsetType>(InCol) * ParityShift<OffsetType>(InCircularRadius);
			}
			else
			{
				return InCol * ColBasis<OffsetType>(InCircularRadius) + InRow * RowBasis<OffsetType>(InCircularRadius) + Parity<OffsetType>(InRow) * ParityShift<OffsetType>(InCircularRadius);
			}
		}
		
		template<EHexOffsetLayout OffsetType>
		FOffsetRealCoord GetHexOffsetPos(const FOffsetCoord InOffsetCoord, float InCircularRadius)
		{
			if constexpr (bIsFlat<OffsetType>)
			{
				return InOffsetCoord.Right * ColBasis<OffsetType>(InCircularRadius) + InOffsetCoord.Up * RowBasis<OffsetType>(InCircularRadius) + Parity<OffsetType>(InOffsetCoord.Right) * ParityShift<OffsetType>(InCircularRadius);
			}
			else
			{
				return InOffsetCoord.Right * ColBasis<OffsetType>(InCircularRadius) + InOffsetCoord.Up * RowBasis<OffsetType>(InCircularRadius) + Parity<OffsetType>(InOffsetCoord.Up) * ParityShift<OffsetType>(InCircularRadius);
			}
		}
	
		template<EHexOffsetLayout OffsetType>
		FOffsetRealCoord WorldSize(uint32 NumCols, uint32 NumRows, float R)
		{
			const HexReal H = sqrt3 * R;      // sqrt(3)*R
			const HexReal W = sqrt3 * R;      // sqrt(3)*R

			if constexpr (bIsFlat<OffsetType>)
			{
				// flat-top: stepX = 1.5R, halfExtentX = R
				const HexReal SizeRight = (NumCols - 1) * 1.5 * R + 2.0 * R;

				// height: stepY = H, плюс запас на паритет (+0.5H) и вершины (+0.5H)
				// Консервативно: H*(NumRows + 0.5)
				const HexReal SizeUp = NumRows * H + 0.5 * H;

				return FOffsetRealCoord(SizeRight, SizeUp);
			}
			else
			{
				// pointy-top: stepY = 1.5R, halfExtentY = R
				const HexReal SizeUp = (NumRows - 1) * 1.5 * R + 2.0 * R;

				// width: stepX = W, плюс возможный паритет (+0.5W) и halfExtent (+0.5W)
				// Консервативно: W*(NumCols + 0.5)
				const HexReal SizeRight = NumCols * W + 0.5 * W;

				return FOffsetRealCoord(SizeRight, SizeUp);
			}
		}

		template<EHexOffsetLayout OffsetType>
		FOffsetRealCoord GetChunkPitchNoGap(uint32 NumCols, uint32 NumRows, float R)
		{
			if constexpr (bIsFlat<OffsetType>)
			{
				// flat-top q-offset:
				// X step per col = 1.5R
				// Y step per row = sqrt(3)R
				return FOffsetRealCoord(1.5 * R * NumCols, GetH(R) * NumRows);
			}
			else
			{
				// pointy-top r-offset:
				// X step per col = sqrt(3)R
				// Y step per row = 1.5R
				return FOffsetRealCoord(GetW(R) * NumCols, 1.5 * R * NumRows);
			}
		}
		
		inline FOffsetCoord OffsetCellToChunk(const FOffsetCoord& InCellCoord, HexInt NumCols, HexInt NumRows)
		{
			if (!ensure(NumCols > 0 && NumRows > 0)) 
				return FOffsetCoord(0,0);
			
			HexInt Rc = InCellCoord.Right / NumCols;
			HexInt Ri = InCellCoord.Right % NumCols;
			if (Ri < 0) Rc--;
	
			HexInt Upc = InCellCoord.Up / NumRows;
			HexInt Upi = InCellCoord.Up % NumRows;
			if (Upi < 0) Upc--;
	
			FOffsetCoord ChunkCoord(Rc,Upc);
			return ChunkCoord;
		}

		inline FVector OffsetHexToWorld(const FOffsetRealCoord& P, float Z = 0)
		{
			return FVector(P.Up, P.Right, Z);
		}
	
		inline FOffsetRealCoord WorldToOffsetHex(const FVector& P)
		{
			return FOffsetRealCoord(P.Y, P.X);
		}
	
		template<EHexOffsetLayout OffsetType>
		FOffsetRealCoord SnapPointShift(float R)
		{
			const HexReal H = sqrt3 * R;
			const HexReal W = sqrt3 * R;

			if constexpr (bIsFlat<OffsetType>)
			{
				return FOffsetRealCoord(0.5 * R, 0.5 * H);
			}
			else
			{
				return FOffsetRealCoord(0.5 * W, 0.5 * R);
			}
		}
	
		template<EHexOffsetLayout OffsetType>
		FVector HexToWorldSnapped(const FOffsetRealCoord& P, float R, float Z = 0)
		{
			const FOffsetRealCoord LB   = SnapPointShift<OffsetType>(R);
			return FVector(P.Up + LB.Up, P.Right + LB.Right, Z);
		}
	
		template<EHexOffsetLayout OffsetType>
		FOffsetRealCoord WorldToHexSnapped(const FVector& P, float R)
		{
			const FOffsetRealCoord LB   = SnapPointShift<OffsetType>(R);
			return FOffsetRealCoord(P.Y - LB.Right, P.X - LB.Up);
		}

	
		template<EHexOffsetLayout OffsetType>
		void BuildHexagonGrid(uint32 InHexLength, float InCircularRadius, 
			const TFunctionRef<void(const FOffsetCoord&, const FVector&)>& InHandler, float InZOffset = 0)
		{
			if (!ensure(InHexLength > 1))
				return;
			if (!ensure(InCircularRadius > 0))
				return;
		
			for (uint32 Row = 0; Row < InHexLength; ++Row)
			{
				for (uint32 Col = 0; Col < InHexLength; ++Col)
				{
					FOffsetCoord OC(Col, Row);
					FOffsetRealCoord FC = GetHexOffsetPos<OffsetType>(OC, InCircularRadius);
					const FVector Loc = HexToWorldSnapped<OffsetType>(FC, InCircularRadius, InZOffset);
					InHandler(OC, Loc);
				}
			}
		}	
	}
}