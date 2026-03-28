// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class GEMSTAR_Interfaces : ModuleRules
{
	public GEMSTAR_Interfaces(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        string ThridPartyLibPath = Path.Combine(PluginDirectory, "ThirdParty");

        // #################################################### OSI 3 ##################################################
        // fill in path to osi3 lib
        string Osi3BasePath = Path.Combine(ThridPartyLibPath, "osi3");
        string Osi3LibPath = Path.Combine(Osi3BasePath, "lib", "osi3");
        string Osi3IncludePath = Path.Combine(Osi3BasePath, "include", "osi3");


        // #################################################### Google Protobuf ##################################################
        // Path to protobuf-ThridParty folder
        string ProtobufBasePath = Path.Combine(ThridPartyLibPath, "protobuf");
        // Path to Lib
        string ProtobufLibPath = Path.Combine(ProtobufBasePath, "lib");
        // Path to protobuf include
        string ProtobufIncludeBasePath = Path.Combine(ProtobufBasePath, "include");

        PublicDefinitions.Add("GOOGLE_PROTOBUF_NO_RTTI");
        PublicDefinitions.Add("GOOGLE_PROTOBUF_INTERNAL_DONATE_STEAL_INLINE=0");
        bEnableUndefinedIdentifierWarnings = false;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // osi3 lib
            PublicAdditionalLibraries.Add(Path.Combine(Osi3LibPath, "open_simulation_interface_static.lib"));
            // protobuf lib
            PublicAdditionalLibraries.Add(Path.Combine(ProtobufLibPath, "libprotobuf.lib"));

        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            // OSI static lib (built from source, placed in ThirdParty tree)
            PublicAdditionalLibraries.Add(Path.Combine(Osi3LibPath, "libopen_simulation_interface_static.a"));

            // Protobuf 3.21.12 (built from source into ~/local/protobuf-3.21.12)
            string ProtobufLocalLib = Path.Combine(
                System.Environment.GetFolderPath(System.Environment.SpecialFolder.UserProfile),
                "local", "protobuf-3.21.12", "lib");
            PublicAdditionalLibraries.Add(Path.Combine(ProtobufLocalLib, "libprotobuf.a"));

            // Use local protobuf headers (system version is too old)
            string ProtobufLocalInclude = Path.Combine(
                System.Environment.GetFolderPath(System.Environment.SpecialFolder.UserProfile),
                "local", "protobuf-3.21.12", "include");
            PublicIncludePaths.Add(ProtobufLocalInclude);
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {

            //PublicAdditionalLibraries.Add(Path.Combine(OsiPath, "lib", "libasam_osilib.dylib"));
        }

        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				Osi3IncludePath,
                ProtobufIncludeBasePath,
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
