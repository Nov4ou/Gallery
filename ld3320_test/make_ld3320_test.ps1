$ErrorActionPreference = "Stop"

# Output settings
$outputDir = "C:\ld3320_test"
$voice = "zh-CN-XiaoxiaoNeural"
$silenceSeconds = "1.5"

$firstText = "你好"
$secondText = "开始"

$firstMp3 = Join-Path $outputDir "nihao.mp3"
$secondMp3 = Join-Path $outputDir "kaishi.mp3"
$silenceWav = Join-Path $outputDir "silence_1p5s.wav"
$finalMp3 = Join-Path $outputDir "ld3320_nihao_kaishi_1p5s.mp3"

# Create output directory
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

# Find Python
$pythonCmd = Get-Command python -ErrorAction SilentlyContinue

if ($null -eq $pythonCmd) {
    $pythonCmd = Get-Command py -ErrorAction SilentlyContinue
}

if ($null -eq $pythonCmd) {
    throw "Python was not found. Please install Python or add it to PATH."
}

$python = $pythonCmd.Source

# Check edge_tts
& $python -c "import edge_tts" 2>$null

if ($LASTEXITCODE -ne 0) {
    throw "edge-tts is not installed for this Python. Run: pip install edge-tts"
}

# Find FFmpeg
$ffmpegCmd = Get-Command ffmpeg -ErrorAction SilentlyContinue

if ($null -ne $ffmpegCmd) {
    $ffmpeg = $ffmpegCmd.Source
}
else {
    $ffmpegSearchResult = Get-ChildItem -Path `
        "$env:LOCALAPPDATA\Microsoft\WinGet\Packages", `
        "$env:ProgramFiles\WinGet\Packages", `
        "C:\Program Files", `
        "C:\Program Files (x86)" `
        -Filter ffmpeg.exe `
        -Recurse `
        -ErrorAction SilentlyContinue |
        Where-Object {
            $_.FullName -like "*Gyan.FFmpeg*" -or $_.FullName -like "*ShareX*"
        } |
        Select-Object -First 1

    if ($null -eq $ffmpegSearchResult) {
        throw "ffmpeg.exe was not found. Please install FFmpeg first."
    }

    $ffmpeg = $ffmpegSearchResult.FullName
}

Write-Host "Using Python: $python"
Write-Host "Using FFmpeg: $ffmpeg"

# Generate TTS audio
Write-Host "Generating TTS: $firstText"
& $python -m edge_tts --voice $voice --text $firstText --write-media $firstMp3

Write-Host "Generating TTS: $secondText"
& $python -m edge_tts --voice $voice --text $secondText --write-media $secondMp3

# Generate silence
Write-Host "Generating $silenceSeconds seconds silence"
& $ffmpeg -y `
    -f lavfi `
    -i "anullsrc=r=44100:cl=mono" `
    -t $silenceSeconds `
    $silenceWav

# Merge final MP3
Write-Host "Merging final MP3"
& $ffmpeg -y `
    -i $firstMp3 `
    -i $silenceWav `
    -i $secondMp3 `
    -filter_complex "[0:a]aformat=sample_rates=44100:channel_layouts=mono[a0];[1:a]aformat=sample_rates=44100:channel_layouts=mono[a1];[2:a]aformat=sample_rates=44100:channel_layouts=mono[a2];[a0][a1][a2]concat=n=3:v=0:a=1[out]" `
    -map "[out]" `
    -codec:a libmp3lame `
    -b:a 192k `
    $finalMp3

Write-Host ""
Write-Host "Done!"
Write-Host "Final file:"
Write-Host $finalMp3