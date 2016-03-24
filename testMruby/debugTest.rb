puts "Start Test"
$lb = LibVision.new()
$lb.set_value4key(["savedImagePath", "default.png"])
#puts $lb.get_value4key(["savedImagePath"]) 
$lb.set_value4key(["imagePath", "/home/pi/testIn.png"])
$lb.execute(["loadImageFromMem"])
$lb.execute(["saveFrame"])
$lb.execute(["preprocessingADPT"])
$lb.execute(["detectSquares"])
$lb.execute(["drawCandidates"])
$lb.execute(["saveFrame"])
$lb.execute(["saveFrame"])
$lb.execute(["loadImageFromMem"])
$lb.execute(["preprocessingADPT"])
$lb.execute(["detectCircles"])
$lb.execute(["drawCandidates"])
$lb.execute(["saveFrame"])

puts "start"
=begin
for i in 0..1500
  #puts i
  $lb.testDebug
end
=end
for i in 0..150 
  puts i
  #$lb.set_value4key(["savedImagePath", "default.png"])
  #puts $lb.get_value4key(["savedImagePath"]) 
  #$lb.set_value4key(["imagePath", "/home/pi/testIn.png"])
  $lb.execute(["loadImageFromMem"])
  $lb.execute(["preprocessingADPT"])
  $lb.execute(["detectSquares"])
  $lb.execute(["drawCandidates"])
  $lb.execute(["saveFrame"])
  $lb.execute(["preprocessingADPT"])
  $lb.execute(["detectCircles"])
  $lb.execute(["drawCandidates"])
end
puts "end"

