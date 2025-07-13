-- Lua script for DaVinci Resolve to add a button to the main toolbar and open an app
-- NOTE: This script must be run from the DaVinci Resolve Script Console or installed
-- ..... in the appropriate Script folder.

-- Load Resolve API
resolve    = Resolve()

-- ui related
ui         = app.UIManager
dispatcher = bmd.UIDispatcher(ui)

-- Create main window
win = dispatcher:AddWindow({
	ID = "DRFXBuilder",
	TargetID = "Fusion", -- Change to place button other than Fusion i.e. 'Edit'
	WindowTitle = "EoF DRFX Bundle Builder",
	Geometry = {100, 100, 400, 80},

	ui:VGroup{
		ID = "root",
		Weight = 1,

		ui:HGroup{
			Weight = 1,
			ui:Button{
				ID = "openDRFXBuilder",
				Text = "Open DRFX Bundle Builder"
			}
		}
	}
})

-- On button click handler
function win.On.openDRFXBuilder.Clicked(ev)
	print("Button clicked. Attempting to open macOS app...")

	-- macOS application path (change as needed)
	local appPath = "/Applications/DRFXBuilder.app"

	-- Use os.execute to open the application
	-- local result = os.execute("open " .. string.format('%q', appPath))

	if result then
		print("Application launched successfully.")
	else
		print("Failed to launch application.")
	end

	dispatcher:ExitLoop()
end

-- Show the window
win:Show()

-- event loop
dispatcher:RunLoop()

-- Clean up after the window is closed
win:Hide()
