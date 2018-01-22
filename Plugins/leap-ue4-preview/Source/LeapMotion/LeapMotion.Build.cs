// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using System.IO;

namespace UnrealBuildTool.Rules
{
	public class LeapMotion : ModuleRules
	{
        private string ModulePath
        {
            get { return ModuleDirectory; }
        }

        private string ThirdPartyPath
        {
            get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
        }

        private string BinariesPath
        {
            get { return Path.GetFullPath(Path.Combine(ModulePath, "../../Binaries/")); }
        }

        private string LibraryPath
        {
            get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "LeapSDK","Lib")); }
        }

		public LeapMotion(ReadOnlyTargetRules Target) : base(Target)
        {
			PublicIncludePaths.AddRange(
				new string[] {
					"LeapMotion/Public",
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"LeapMotion/Private",
                    Path.Combine(ThirdPartyPath, "LeapSDK", "Include"),
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Engine",
					"Core",
                    "CoreUObject",
                    "InputCore",
                    "InputDevice",
                    "Slate",
                    "SlateCore",
                    "HeadMountedDisplay",
                    "RHI",
                    "RenderCore",
                    "Projects",
                    "BodyState"
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);

            LoadLeapLib(Target);
		}

        public string GetUProjectPath()
        {
            return Path.Combine(ModuleDirectory, "../../../..");
        }

        private void CopyToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
        {
            System.Console.WriteLine("uprojectpath is: " + Path.GetFullPath(GetUProjectPath()));

            string binariesDir = Path.Combine(GetUProjectPath(), "Binaries", Target.Platform.ToString());
            string filename = Path.GetFileName(Filepath);

            //convert relative path
            string fullBinariesDir = Path.GetFullPath(binariesDir);

            if (!Directory.Exists(fullBinariesDir))
                Directory.CreateDirectory(fullBinariesDir);

            if (!File.Exists(Path.Combine(fullBinariesDir, filename)))
            {
                System.Console.WriteLine("LeapPlugin: Copied from " + Filepath + ", to " + Path.Combine(fullBinariesDir, filename));
                File.Copy(Filepath, Path.Combine(fullBinariesDir, filename), true);
            }
        }

        public bool LoadLeapLib(ReadOnlyTargetRules Target)
        {
            bool isLibrarySupported = false;
            bool isProjectPlugin = true;    //disable for engine plugin

            if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
            {
                isLibrarySupported = true;

                string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";

                //Lib
                PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "LeapC.lib"));

                //DLL
                string PluginDLLPath = Path.Combine(BinariesPath, PlatformString, "LeapC.dll");
                if (isProjectPlugin)
                {
                    //For project plugins, copy the dll to the project if needed
                    CopyToProjectBinaries(PluginDLLPath, Target);

                    string DLLPath = Path.GetFullPath(Path.Combine(GetUProjectPath(), "Binaries", PlatformString, "LeapC.dll"));
                    RuntimeDependencies.Add(new RuntimeDependency(DLLPath));
                }
                //Engine plugin, just add the dependency path
                else
                {
                    RuntimeDependencies.Add(new RuntimeDependency(PluginDLLPath)); 
                }                
            }
            else if (Target.Platform == UnrealTargetPlatform.Mac){

                isLibrarySupported = true;

                string PlatformString = "Mac";
                PublicAdditionalLibraries.Add(Path.Combine(BinariesPath, PlatformString, "libLeap.dylib"));

            }
            else if (Target.Platform == UnrealTargetPlatform.Android)
            {
                System.Console.WriteLine(Target.Architecture);    //doesn't work

                string PlatformString = "Android";

                //For now comment/uncomment platform architectures
                //PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "armeabi-v7a", "libLeapC.so"));
                PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, PlatformString, "arm64-v8a", "libLeapC.so"));

                AdditionalPropertiesForReceipt.Add(new ReceiptProperty("AndroidPlugin", Path.Combine(ModulePath, "LeapMotion_APL.xml")));
                
                isLibrarySupported = true;
            }

            return isLibrarySupported;
        }
	}
}