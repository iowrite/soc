#! /usr/bin/fish
echo "please input version:"
read ver
zip -r  SOX_$ver.zip src/ README.md  doc/ CHANGELOG 