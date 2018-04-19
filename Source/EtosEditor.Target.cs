// © 2016 - 2017 Daniel Bortfeld

using UnrealBuildTool;
using System.Collections.Generic;

public class EtosEditorTarget : TargetRules
{
    public EtosEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;

        ExtraModuleNames.AddRange(new string[] { "Etos" });
    }

    //
    // TargetRules interface.
    //

    //public override void SetupBinaries(
    //    TargetInfo Target,
    //    ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
    //    ref List<string> OutExtraModuleNames
    //    )
    //{
    //    OutExtraModuleNames.AddRange(new string[] { "Etos" });
    //}
}

//"Plugins": [
//    {
//        "Name": "Substance",
//        "Enabled": true,
//        ""
//    }
//],