print("Hello World!")

knob1 = 120 
knob2 = 120 
knob3 = 120 
knob4 = 120 
knob5 = 120

----------------------------------------------------
function setup()
	of.setWindowTitle("knobs example")
	print("script setup")

	of.setCircleResolution(50)
	of.background(0,0,0)
	
	of.setFrameRate(60) -- if vertical sync is off, we can go a bit fast... this caps the framerate at 60fps
	of.disableSmoothing()
	--of.disableAlphaBlending()
end

----------------------------------------------------
function update()
end

----------------------------------------------------
function draw()

	of.fill()
   
    of.background(33, 33, 33)
    of.setColor(0, 255, 100)
	of.drawCircle(knob1, knob2, knob3)

end

----------------------------------------------------
function exit()
	print("script finished")
end



