@Echo Off
setlocal enableDelayedExpansion
REM ## Create the GameMode Power Plan Via Batch ##
IF "%~1" EQU "" (
set gm=b8e6d75e-26e8-5e8f-efef-e94a209a3467
set name=Game Mode
) ELSE (
set gm=%~1
set name=%~2
)
call :SANITYCHK
powercfg /DUPLICATESCHEME "8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c" "!gm!" >nul 2>&1
powercfg /CHANGENAME "!gm!" "!name!"
REM ## Start Configuring Default Windows Power Plan ##
powercfg /SETACVALUEINDEX "!gm!" SUB_DISK DISKIDLE 2100
powercfg /SETACVALUEINDEX "!gm!" 02f815b5-a5cf-4c84-bf20-649d1f75d3d8 4c793e7d-a264-42e1-87d3-7a0d2f523ccd 1
powercfg /SETACVALUEINDEX "!gm!" 0d7dbae2-4294-402a-ba8e-26777e8488cd 309dce9b-bef4-4119-9921-a851fb12f0f4 0
powercfg /SETACVALUEINDEX "!gm!" 19cbb8fa-5279-450e-9fac-8a3d5fedd0c1 12bbebe6-58d6-4636-95bb-3217ef867c1a 0
powercfg /SETACVALUEINDEX "!gm!" SUB_SLEEP STANDBYIDLE 3600
powercfg /SETACVALUEINDEX "!gm!" SUB_SLEEP HYBRIDSLEEP 0
powercfg /SETACVALUEINDEX "!gm!" SUB_SLEEP HIBERNATEIDLE 0
powercfg /SETACVALUEINDEX "!gm!" SUB_SLEEP RTCWAKE 2
IF !ERRORLEVEL! NEQ 0 (powercfg /SETACVALUEINDEX "!gm!" 238c9fa8-0aad-41ed-83f4-97be242c8f20 bd3b718a-0680-4d9d-8ab2-e1d2b4ac806d 1)
powercfg /SETACVALUEINDEX "!gm!" 2a737441-1930-4402-8d77-b2bebba308a3 48e6b7a6-50f5-4782-a5d4-53bb8f07e226 1
powercfg /SETACVALUEINDEX "!gm!" SUB_BUTTONS UIBUTTON_ACTION 0
powercfg /SETACVALUEINDEX "!gm!" SUB_PCIEXPRESS ASPM 0
powercfg /SETACVALUEINDEX "!gm!" SUB_PROCESSOR PROCTHROTTLEMIN 100
powercfg /SETACVALUEINDEX "!gm!" SUB_PROCESSOR PROCTHROTTLEMAX 100
powercfg /SETACVALUEINDEX "!gm!" SUB_PROCESSOR SYSCOOLPOL 1
IF !ERRORLEVEL! NEQ 0 (powercfg /SETACVALUEINDEX "!gm!" 54533251-82be-4824-96c1-47b60b740d00 94d3a615-a899-4ac5-ae2b-e4d8f634367f 1)
powercfg /SETACVALUEINDEX "!gm!" SUB_VIDEO VIDEOIDLE 900
powercfg /SETACVALUEINDEX "!gm!" SUB_VIDEO aded5e82-b909-4619-9949-f5d71dac0bcb 100
powercfg /SETACVALUEINDEX "!gm!" SUB_VIDEO f1fbfde2-a960-4165-9f88-50667911ce96 50
powercfg /SETACVALUEINDEX "!gm!" SUB_VIDEO ADAPTBRIGHT 0
IF !ERRORLEVEL! NEQ 0 (powercfg /SETACVALUEINDEX "!gm!" 7516b95f-f776-4464-8c53-06167f40cc99 fbd9aa66-9553-4097-ba44-ed6e9d65eab8 0)
powercfg /SETACVALUEINDEX "!gm!" 9596fb26-9850-41fd-ac3e-f7c3c00afd4b 03680956-93bc-4294-bba6-4e0f09bb717f 1
powercfg /SETACVALUEINDEX "!gm!" 9596fb26-9850-41fd-ac3e-f7c3c00afd4b 10778347-1370-4ee0-8bbd-33bdacaade49 1
powercfg /SETACVALUEINDEX "!gm!" 9596fb26-9850-41fd-ac3e-f7c3c00afd4b 34c7b99f-9a6d-4b3c-8dc7-b6693b78cef4 0
powercfg /SETACVALUEINDEX "!gm!" 7516b95f-f776-4464-8c53-06167f40cc99 17aaa29b-8b43-4b94-aafe-35f64daaf1ee 600
::GPU Power Policy
powercfg /SETACVALUEINDEX "!gm!" 5fb4938d-1ee8-4b0f-9a3c-5036b0ab995c dd848b2a-8a5d-4451-9ae2-39cd41658f6c 0
powercfg /SETDCVALUEINDEX "!gm!" 5fb4938d-1ee8-4b0f-9a3c-5036b0ab995c dd848b2a-8a5d-4451-9ae2-39cd41658f6c 0
REM ## Start DC Power Plan ##
powercfg /SETDCVALUEINDEX "!gm!" SUB_DISK DISKIDLE 2100
powercfg /SETDCVALUEINDEX "!gm!" 02f815b5-a5cf-4c84-bf20-649d1f75d3d8 4c793e7d-a264-42e1-87d3-7a0d2f523ccd 1
powercfg /SETDCVALUEINDEX "!gm!" 0d7dbae2-4294-402a-ba8e-26777e8488cd 309dce9b-bef4-4119-9921-a851fb12f0f4 0
powercfg /SETDCVALUEINDEX "!gm!" 19cbb8fa-5279-450e-9fac-8a3d5fedd0c1 12bbebe6-58d6-4636-95bb-3217ef867c1a 0
powercfg /SETDCVALUEINDEX "!gm!" SUB_SLEEP STANDBYIDLE 2700
powercfg /SETDCVALUEINDEX "!gm!" SUB_SLEEP HYBRIDSLEEP 0
powercfg /SETDCVALUEINDEX "!gm!" SUB_SLEEP HIBERNATEIDLE 0
powercfg /SETDCVALUEINDEX "!gm!" SUB_SLEEP RTCWAKE 2
IF !ERRORLEVEL! NEQ 0 (powercfg /SETDCVALUEINDEX "!gm!" 238c9fa8-0aad-41ed-83f4-97be242c8f20 bd3b718a-0680-4d9d-8ab2-e1d2b4ac806d 1)
powercfg /SETDCVALUEINDEX "!gm!" 2a737441-1930-4402-8d77-b2bebba308a3 48e6b7a6-50f5-4782-a5d4-53bb8f07e226 1
powercfg /SETDCVALUEINDEX "!gm!" SUB_BUTTONS UIBUTTON_ACTION 0
powercfg /SETDCVALUEINDEX "!gm!" SUB_PCIEXPRESS ASPM 0
powercfg /SETDCVALUEINDEX "!gm!" SUB_PROCESSOR PROCTHROTTLEMIN 100
powercfg /SETDCVALUEINDEX "!gm!" SUB_PROCESSOR PROCTHROTTLEMAX 100
powercfg /SETDCVALUEINDEX "!gm!" SUB_PROCESSOR SYSCOOLPOL 1
IF !ERRORLEVEL! NEQ 0 (powercfg /SETDCVALUEINDEX "!gm!" 54533251-82be-4824-96c1-47b60b740d00 94d3a615-a899-4ac5-ae2b-e4d8f634367f 1)
powercfg /SETDCVALUEINDEX "!gm!" SUB_VIDEO VIDEOIDLE 900
powercfg /SETDCVALUEINDEX "!gm!" SUB_VIDEO aded5e82-b909-4619-9949-f5d71dac0bcb 100
powercfg /SETDCVALUEINDEX "!gm!" SUB_VIDEO f1fbfde2-a960-4165-9f88-50667911ce96 50
powercfg /SETDCVALUEINDEX "!gm!" SUB_VIDEO ADAPTBRIGHT 0
IF !ERRORLEVEL! NEQ 0 (powercfg /SETDCVALUEINDEX "!gm!" 7516b95f-f776-4464-8c53-06167f40cc99 fbd9aa66-9553-4097-ba44-ed6e9d65eab8 0)
powercfg /SETDCVALUEINDEX "!gm!" 9596fb26-9850-41fd-ac3e-f7c3c00afd4b 03680956-93bc-4294-bba6-4e0f09bb717f 1
powercfg /SETDCVALUEINDEX "!gm!" 9596fb26-9850-41fd-ac3e-f7c3c00afd4b 10778347-1370-4ee0-8bbd-33bdacaade49 1
powercfg /SETDCVALUEINDEX "!gm!" 9596fb26-9850-41fd-ac3e-f7c3c00afd4b 34c7b99f-9a6d-4b3c-8dc7-b6693b78cef4 0
powercfg /SETDCVALUEINDEX "!gm!" 7516b95f-f776-4464-8c53-06167f40cc99 17aaa29b-8b43-4b94-aafe-35f64daaf1ee 300

REM ####################### START OEM SPECIFIC PowerPlan Editing ##################################################################################################
::AMD Performance Slider
powercfg /SETACVALUEINDEX "!gm!" c763b4ec-0e50-4b6b-9bed-2b92a6ee884e 7ec1751b-60ed-4588-afb5-9819d3d77d90 3
powercfg /SETDCVALUEINDEX "!gm!" c763b4ec-0e50-4b6b-9bed-2b92a6ee884e 7ec1751b-60ed-4588-afb5-9819d3d77d90 3
::AMD Switchable Dynamic Graphics To Dedicated GPU
powercfg /SETACVALUEINDEX "!gm!" e276e160-7cb0-43c6-b20b-73f5dce39954 a1662ab2-9d34-4e53-ba8b-2639b9e20857 3
powercfg /SETDCVALUEINDEX "!gm!" e276e160-7cb0-43c6-b20b-73f5dce39954 a1662ab2-9d34-4e53-ba8b-2639b9e20857 3
::AMD PowerPlay Settings Max Performance
powercfg /SETACVALUEINDEX "!gm!" f693fb01-e858-4f00-b20f-f30e12ac06d6 191f65b5-d45c-4a4f-8aae-1ab8bfd980e6 1
powercfg /SETDCVALUEINDEX "!gm!" f693fb01-e858-4f00-b20f-f30e12ac06d6 191f65b5-d45c-4a4f-8aae-1ab8bfd980e6 1
::INTEL HD Integrated Graphics
powercfg /SETACVALUEINDEX "!gm!" 44f3beca-a7c0-460e-9df2-bb8b99e0cba6 3619c3f2-afb2-4afc-b0e9-e7fef372de36 2
powercfg /SETDCVALUEINDEX "!gm!" 44f3beca-a7c0-460e-9df2-bb8b99e0cba6 3619c3f2-afb2-4afc-b0e9-e7fef372de36 2

REM ## Set the active Power Plan to be Game Mode Power Plan ##
powercfg /SETACTIVE "!gm!"
exit /b

:SANITYCHK
set sanity=T
powercfg query "8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (set sanity=F)
powercfg query "381b4222-f694-41f0-9685-ff5bb260df2e" >nul 2>&1
IF !ERRORLEVEL! NEQ 0 (set sanity=F)
IF "!sanity!" NEQ "T" (
echo Restoring Power Plans High Performance and Balanced
call :RSTPWR
)
exit /b

:RSTPWR
set batdir=%TMP%\TMPBAT
set dummypp=0319040a-eec8-a92b-85c8-e6f1958b6fdd
del /F /Q /A !batdir! >nul 2>&1
RD !batdir! >nul 2>&1
mkdir "!batdir!"
FOR /F "tokens=2* delims=:" %%I IN ('powercfg ^/L') DO (
FOR /F %%D IN ("%%I") DO (
set guid=%%D
powercfg /EXPORT "!batdir!\!guid!" "!guid!"
)
)
REM ## Re-Create Default Power Plans ##
powercfg -RESTOREDEFAULTSCHEMES
REM ## Create Dummy TMP Plan ##
FOR /F "tokens=3*" %%A IN ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes" /v "ActivePowerScheme"') DO (set PrevPowerPlan=%%A)
powercfg /DUPLICATESCHEME SCHEME_CURRENT "!dummypp!" >nul 2>&1
powercfg /SETACTIVE "!dummypp!" >nul 2>&1
REM ## RE-IMPORT Everything ##
FOR /F "delims=" %%I IN ('dir /A /B !batdir!') DO (
powercfg /DELETE "%%I" >nul 2>&1
powercfg /IMPORT "!batdir!\%%I" "%%I" 2>nul
IF !ERRORLEVEL! EQU 0 (
echo[
)
)
REM ## Restore PowerPlan and delete the Dummy ##
powercfg /SETACTIVE "!PrevPowerPlan!" >nul 2>&1
powercfg /DELETE "!dummypp!" >nul 2>&1
del /F /Q /A !batdir! >nul 2>&1
RD !batdir! >nul 2>&1
exit /b
