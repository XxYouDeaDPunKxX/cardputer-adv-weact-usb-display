param(
 [string]$Cli='arduino-cli',
 [string]$Config='',
 [string]$EspUsbHostDir=''
)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$demo=Join-Path $root 'demo\CardputerADV_WeActDemo'
$library=Join-Path $root 'library\WeActUSBDisplay'
$build=Join-Path $root '.build\arduino-cache'
$out=Join-Path $root '.build\arduino-output'
if(!(Test-Path -LiteralPath $demo -PathType Container)){throw 'Demo folder missing'}
if(!(Test-Path -LiteralPath $library -PathType Container)){throw 'Library folder missing'}
if($Config -and !(Test-Path -LiteralPath $Config -PathType Leaf)){throw 'CLI config missing'}
if($EspUsbHostDir){
 if(!(Test-Path -LiteralPath (Join-Path $EspUsbHostDir 'library.properties') -PathType Leaf)){
  throw 'EspUsbHost library.properties missing'
 }
 if(!(Select-String -LiteralPath (Join-Path $EspUsbHostDir 'library.properties') -Pattern '^version=2\.10\.1\s*$')){
  throw 'Expected EspUsbHost 2.10.1'
 }
}
$fqbn='esp32:esp32:m5stack_cardputer:USBMode=hwcdc,CDCOnBoot=cdc,FlashSize=8M,PartitionScheme=custom,PSRAM=disabled'
$args=@()
if($Config){$args+=@('--config-file',$Config)}
$args+=@('compile','--fqbn',$fqbn,'--library',$library,
         '--build-path',$build,'--output-dir',$out)
if($EspUsbHostDir){$args+=@('--library',$EspUsbHostDir)}
$homePath=[Environment]::GetFolderPath('UserProfile')
if($homePath){
 $prefixMap="-ffile-prefix-map=$homePath=/source -ffile-prefix-map=$($homePath.Replace('\','/'))=/source"
 $args+=@('--build-property',"compiler.c.extra_flags=$prefixMap",
          '--build-property',"compiler.cpp.extra_flags=$prefixMap")
}
$args+=$demo
Write-Output "Compiling Cardputer ADV demo; no upload or flash."
& $Cli @args
if($LASTEXITCODE -ne 0){throw "Arduino compilation failed: $LASTEXITCODE"}
Write-Output "BUILD PASS. Output: $out"
Write-Output 'Test this build on hardware before distributing it.'

