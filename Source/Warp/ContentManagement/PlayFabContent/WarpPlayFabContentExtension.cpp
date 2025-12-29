namespace WarpPlayfabContent
{
	TOptional<FString> ReadSecret()
	{
		const FString PlayfabKeysPath(TEXT("PlayfabKeys"));
		const FString PathValue = FPlatformMisc::GetEnvironmentVariable(*PlayfabKeysPath);

		FString Secret;
		if (GConfig->GetString(TEXT("PlayFab"), TEXT("SecretKey"), Secret, PathValue))
		{
			Secret.TrimStartAndEndInline();
			Secret.ReplaceInline(TEXT("\""), TEXT(""));

			if (!Secret.IsEmpty())
			{
				return TOptional<FString>(MoveTemp(Secret)); // UE-style move
			}
		}

		return {};
	}
}