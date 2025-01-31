<# if($args[0]) {
    Add-Type -AssemblyName UIAutomationClient
    $MyProcess = Get-Process | where { $_.Id -eq $PID }
    $ae = [System.Windows.Automation.AutomationElement]::FromHandle($MyProcess.MainWindowHandle)
    $wp = $ae.GetCurrentPattern([System.Windows.Automation.WindowPatternIdentifiers]::Pattern)
    $wp.SetWindowVisualState("Minimized")
} #>
if(-not $args[0])
{
    $currentWi = [Security.Principal.WindowsIdentity]::GetCurrent()
    $currentWp = [Security.Principal.WindowsPrincipal]$currentWi
    if(-not $currentWp.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator))
    {
        $boundPara = ($MyInvocation.BoundParameters.Keys | foreach { '-{0} {1}' -f  $_ ,$MyInvocation.BoundParameters[$_] }) -join ' '
        $currentFile = (Resolve-Path  $MyInvocation.InvocationName).Path
        $fullPara = $boundPara + ' ' + $args -join ' '
        Start-Process "$psHome\powershell.exe"   -ArgumentList "$currentFile $fullPara"   -verb runas
        return
    }
}


$BeginLines = @() # array
$EndLines = @()
$File = Get-Content -Path "C:\Windows\System32\drivers\etc\hosts"
$File | Select-String -Pattern "# github hosts" | ForEach-Object { $BeginLines += $_.LineNumber }
$File | Select-String -Pattern "# github hosts end" | ForEach-Object { $EndLines += $_.LineNumber }
$flag = $true
if($BeginLines.length -eq 0) { $flag = $false }

$OutputFile = ""
if($flag)
{
    $cnt = 0
    foreach($i in $File) {
        ++$cnt
        if(($cnt -ge $BeginLines[0]) -and  ($cnt -le $EndLines[0])) { continue; echo $i }
        else { $OutputFile += $i; $OutputFile += "`n" }
    }
}
else {
    foreach($i in $File) {
        $OutputFile += $i
        $OutputFile += "`n"
    }
}

# https://hosts.gitcdn.top/hosts.txt
# https://gitee.com/if-the-wind/github-hosts/raw/main/hosts
# https://github-hosts.tinsfox.com/hosts
$Response = Invoke-WebRequest -URI https://github-hosts.tinsfox.com/hosts
$OutputFile += $Response.Content
$OutputFile += "# github hosts end"

Write-Host "Writing these content to C:\Windows\System32\drivers\etc\hosts:`n$([char]0x1b)[96m$OutputFile"
$OutputFile | Out-file -FilePath "C:\Windows\System32\drivers\etc\hosts"
if($args[0]) { $OutputFile | Out-file -FilePath "$env:USERPROFILE\Desktop\fetch-github-hosts.log" }

ipconfig /flushdns

if(-not $args[0]) { pause }
