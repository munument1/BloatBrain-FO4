Scriptname BBCompanionRecruitScript extends Actor

GlobalVariable Property BB_Recruited Auto Const Mandatory
GlobalVariable Property BB_BridgeOnline Auto Const Mandatory
Message Property BB_RecruitMessage Auto Const Mandatory
Message Property BB_CompanionMenuMessage Auto Const Mandatory

Event OnInit()
    SetProtected(true)
EndEvent

Event OnActivate(ObjectReference akActionRef)
    if akActionRef != Game.GetPlayer()
        return
    endif

    if BB_Recruited.GetValue() < 1.0
        int choice = BB_RecruitMessage.Show()
        if choice == 0
            Recruit()
        endif
    else
        int choice = BB_CompanionMenuMessage.Show()
        if choice == 0
            EvaluatePackage()
        elseif choice == 1
            DismissToRedRocket()
        endif
    endif
EndEvent

Function Recruit()
    BB_Recruited.SetValue(1.0)
    SetRelationshipRank(Game.GetPlayer(), 3)
    SetPlayerTeammate(true, false, false)
    EvaluatePackage()
EndFunction

Function DismissToRedRocket()
    BB_Recruited.SetValue(0.0)
    SetPlayerTeammate(false)
    EvaluatePackage()
EndFunction
