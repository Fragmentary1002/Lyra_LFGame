// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LyraServerSteamTarget : LyraServerTarget
{
	public LyraServerSteamTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "Steam";
		RegisterModulesCreatedByRider();
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[] { "LFGame" });
	}
}
