print("Hello World!")

knob1 = 120 
knob2 = 120 
knob3 = 120 
knob4 = 120 
knob5 = 120

counter = 0
bSmooth = false

cam = of.Camera()

----------------------------------------------------
function setup()
	of.setWindowTitle("graphics example")
	print("script setup")

	of.setCircleResolution(50)
	of.background(0,0,0)
	of.setWindowTitle("graphics example")
	
	of.setFrameRate(60) -- if vertical sync is off, we can go a bit fast... this caps the framerate at 60fps
	of.disableSmoothing()
	--of.disableAlphaBlending()
end

----------------------------------------------------
function update()
	counter = counter + 1
end

----------------------------------------------------
function draw()

    cam:beginCamera()
	of.fill()
    cam:setFov(knob4/2)
    cam:setPosition(knob1, knob2, knob3)
    cam:panDeg(knob5/512)
    --cam:tiltDeg(knob5/512)
    --cam:rollDeg(knob5/512)
    --cam:truck(knob5) --one of the setPosition() functions
    --cam:dolly(knob5) --one of the setPosition() functions
    --cam:boom(knob5)  --one of the setPosition() functions
   
    -- of.background(of.random(0, 255), of.random(0, 255), of.random(0, 255))
    of.setColor(of.random(0, 255), of.random(0, 255), of.random(0, 255))
	of.setPolyMode(of.POLY_WINDING_ODD)
	of.beginShape()
	for i=1,10 do
		of.vertex(of.random(10, 200), of.random(10, 300))
	end
	of.endShape(false)
	
	 --of.setColor(of.random(0, 255), of.random(0, 255), of.random(0, 255), 140)
	 --of.drawCircle(960, 540, 540)
	 of.setColor(10, 200, 100)

    of.drawGrid(20,10,false,true,true,true)

	of.pushMatrix()
		of.translate(960, 640, 0)
		-- of.setHexColor(0xff2220CC)
		of.setColor(200, of.random(0, 255), 0, of.random(0, 255))
		of.fill()
		of.setPolyMode(of.POLY_WINDING_ODD)
		of.beginShape()
		local angleStep 	= of.TWO_PI/(100 + math.sin(of.getElapsedTimef()/5) * 60)
		local radiusAdder 	= .5
		local radius 		= 0
		for i=1,10 do
		    of.vertex(of.random(0, 202), of.random(0, 110))
	    end
		for i=1,500 do
			local anglef = (i) * angleStep
			local x = radius * math.cos(anglef * 4)
			local y = radius * math.sin(anglef * 4) 
			of.vertex(x, y)
			radius 	= radius + radiusAdder
		end
		of.endShape(of.CLOSE)
	of.popMatrix()
	
	of.noFill()
	for i=0,50 do
		of.setColor(of.random(0, 255), of.random(0, 255), of.random(0, 255),200)
		of.drawRectangle(of.random(1000, 2020), of.random(500, 1180),
		                 of.random(1000, 2020), of.random(500, 1180))
	end
	
	cam:endCamera()
	
	of.setHexColor(0xff0000)
	of.drawBitmapString("owenhh!!!!", 75, 500)

end

----------------------------------------------------
function exit()
	print("script finished")
end



