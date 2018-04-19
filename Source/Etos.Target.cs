// © 2016 - 2017 Daniel Bortfeld

using UnrealBuildTool;
using System.Collections.Generic;

public class EtosTarget : TargetRules
{
    public EtosTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;

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
