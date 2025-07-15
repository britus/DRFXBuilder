-- Load Resolve API
local  resolve    = Resolve()

-- ui related
local ui         = app.UIManager
local dispatcher = bmd.UIDispatcher(ui)

-- Create main window
local win = dispatcher:AddWindow({
    ID = "DRFXBuilder",
	TargetID = "Fusion",
	WindowTitle = "EoF DRFX Bundle Builder",
	Geometry = {100, 100, 400, 80},

    ui:VGroup{
	    ID = "root",
		Weight = 1,

        ui:HGroup{
		    Weight = 1,
			ui:Button{
			    ID = "Okay",
				Text = "Can't open DRFX Bundle Builder App"
			}
		}
	}
})

-- On button click handler
function win.On.Okay.Clicked(ev)
    dispatcher:ExitLoop()
end

local appPath = "/Applications/DRFXBuilder.app"
local result = os.execute("open " .. string.format('%q', appPath))

if result then
    print("Application launched successfully.")
else
   win:Show()
   dispatcher:RunLoop()
   win:Hide()
end
