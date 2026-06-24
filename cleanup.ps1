$utf8NoBom = New-Object System.Text.UTF8Encoding $False
$files = Get-ChildItem -Path src -Include *.cpp, *.h -Recurse | Where-Object { $_.Name -ne 'json.hpp' }

$totalComments = 0
$totalBraces = 0

foreach ($file in $files) {
    try {
        $lines = [System.IO.File]::ReadAllLines($file.FullName, $utf8NoBom)
        $modified = $false
        $newLines = New-Object System.Collections.Generic.List[string]

        for ($i = 0; $i -lt $lines.Length; $i++) {
            $line = $lines[$i]
            $skip = $false

            # === PASS 1: Remove comment lines ===

            # Remove lines that are ONLY a comment starting with an English word (no code before //)
            # This catches: // Position terrain slightly lower
            #               // Camera Controls
            #               // Rotation
            #               // Log bounding box ...
            # But NOT: vertex.position.x *= -1.0f; // RH -> LH  (has code before //)
            if ($line -match '^\s*//\s*[A-Za-z]') {
                $skip = $true
            }
            # Remove lines containing the star character
            if ($line -match '\u2605') {
                $skip = $true
            }

            if ($skip) {
                $modified = $true
                $totalComments++
                continue
            }

            # Remove trailing English-only inline comments: code // English text
            # Match: "; // Yaw" or "; // some english"
            # But keep: "; // 最大4つまで" (contains Japanese)
            if ($line -match '^(.+;)\s*//\s*([A-Za-z0-9_\-\(\)\.\s,>:=&|!*+/]+)$') {
                $codePart = $Matches[1]
                $commentPart = $Matches[2]
                # Only remove if the comment contains NO Japanese/CJK characters
                if ($commentPart -notmatch '[\u3000-\u9FFF\uFF00-\uFFEF]') {
                    $line = $codePart
                    $modified = $true
                    $totalComments++
                }
            }

            $newLines.Add($line)
        }

        # === PASS 2: Add braces to if statements ===
        $pass2Lines = New-Object System.Collections.Generic.List[string]
        for ($i = 0; $i -lt $newLines.Count; $i++) {
            $line = $newLines[$i]

            # Single-line if without braces: if (cond) statement;
            # Matches: if (...) return ...; or if (...) continue; etc.
            # Does NOT match: if (...) { ... } (already has braces)
            if ($line -match '^(\s*)((?:else\s+)?if\s*\(.*\))\s+([^\{].*;\s*)$') {
                $indent = $Matches[1]
                $ifPart = $Matches[2]
                $body = $Matches[3].Trim()
                $line = "$indent$ifPart { $body }"
                $modified = $true
                $totalBraces++
            }

            $pass2Lines.Add($line)
        }

        if ($modified) {
            [System.IO.File]::WriteAllLines($file.FullName, $pass2Lines, $utf8NoBom)
            Write-Output "Modified $($file.FullName)"
        }
    }
    catch {
        Write-Error "Error processing $($file.FullName): $_"
    }
}

Write-Output ""
Write-Output "=== Summary ==="
Write-Output "Comments removed: $totalComments"
Write-Output "If-braces added: $totalBraces"
