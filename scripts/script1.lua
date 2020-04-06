print("Hello World!")

knob1 = 120 
knob2 = 120 
knob3 = 120 
knob4 = 120 
knob5 = 120

fbo = of.fbo()

----------------------------------------------------
function setup()
	of.setWindowTitle("knobs example")
	print("script setup")

    fbo:allocate(1280,720 );
	of.setCircleResolution(50)
	of.background(0,0,0)
	of.setFrameRate(60) -- if vertical sync is off, we can go a bit fast... this caps the framerate at 60fps
	of.disableSmoothing()
	--of.SetBackgroundAuto(0);
	--of.disableAlphaBlending()
end

----------------------------------------------------
function update()
end

----------------------------------------------------
function draw()

    
   fbo:beginFbo()
	
	
	of.fill()
	of.setColor(of.random(0, 255), of.random(0, 255), of.random(0, 255))
	of.drawCircle(100, 100, knob1)

	
	
 	fbo:endFbo();
    fbo:draw(0,0);
    
	of.setColor(of.random(0, 255), of.random(0, 255), of.random(0, 255))
	of.drawCircle(100, 100, knob1)

end

----------------------------------------------------
function exit()
	print("script finished")
end



