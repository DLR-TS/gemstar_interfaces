/*
 * Copyright (c) 2026 DLR e.V.
 *
 * This program and the accompanying materials are made
 * available under the terms of the Eclipse Public License 2.0
 * which is available at https://www.eclipse.org/legal/epl-2.0/
 *
 * SPDX-License-Identifier: EPL-2.0
 */
using System.IO;
using UnrealBuildTool;

public class GEMSTAR_Interfaces : ModuleRules
{
	public GEMSTAR_Interfaces(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;

        string ThirdPartyLibPath = Path.Combine(PluginDirectory, "ThirdParty");

        // #################################################### OSI 3 ##################################################
        // fill in path to osi3 lib
        string Osi3BasePath = Path.Combine(ThirdPartyLibPath, "osi3");
        string Osi3LibPath = Path.Combine(Osi3BasePath, "lib", "osi3");
        string Osi3IncludePath = Path.Combine(Osi3BasePath, "include", "osi3");


        // #################################################### Google Protobuf ##################################################
        // Path to protobuf-ThridParty folder
        string ProtobufBasePath = Path.Combine(ThirdPartyLibPath, "protobuf");
        // Path to Lib
        string ProtobufLibPath = Path.Combine(ProtobufBasePath, "lib");
        // Path to protobuf include
        string ProtobufIncludeBasePath = Path.Combine(ProtobufBasePath, "include");

        PublicDefinitions.Add("GOOGLE_PROTOBUF_NO_RTTI");
        PublicDefinitions.Add("GOOGLE_PROTOBUF_INTERNAL_DONATE_STEAL_INLINE=0");

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // osi3 lib
            PublicAdditionalLibraries.Add(Path.Combine(Osi3LibPath, "open_simulation_interface_static.lib"));
            // protobuf lib
            PublicAdditionalLibraries.Add(Path.Combine(ProtobufLibPath, "libprotobuf.lib"));

        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicIncludePaths.Add(Osi3IncludePath);
            PublicIncludePaths.Add(ProtobufIncludeBasePath);

            PublicAdditionalLibraries.Add(
                Path.Combine(Osi3LibPath, "libopen_simulation_interface_pic.a")
            );
            PublicAdditionalLibraries.Add(
                Path.Combine(ProtobufLibPath, "libprotobuf.a")
            );
            PublicSystemLibraries.Add("stdc++");
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
				"CoreUObject",
				"Engine",
                "XmlParser",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"Projects",
				"AssetRegistry",
				"GameFeatures",
				"ModularGameplay"
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
