param([string]$KeilC51Root='D:/keil-C51/C51')
$ErrorActionPreference='Stop'
Push-Location $PSScriptRoot
try {
 foreach($variant in @(@{Name='basic';Improve=0;Pin=0},@{Name='improved_board';Improve=1;Pin=0},@{Name='improved_p26';Improve=1;Pin=1})) {
  & "$KeilC51Root/BIN/C51.exe" main.c "INCDIR($KeilC51Root/INC)" "DEFINE(IMPROVED=$($variant.Improve),TEMP_P26=$($variant.Pin))" OBJECTEXTEND
  if($LASTEXITCODE -gt 1){throw 'Compile failed'}
  & "$KeilC51Root/BIN/BL51.exe" main.obj TO $variant.Name
  if($LASTEXITCODE -gt 1){throw 'Link failed'}
  & "$KeilC51Root/BIN/OH51.exe" $variant.Name
  if($LASTEXITCODE -ne 0){throw 'HEX failed'}
 }
} finally {Pop-Location}
