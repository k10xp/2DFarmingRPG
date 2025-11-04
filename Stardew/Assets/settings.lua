
function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end



function GetSettingsViewModel()
    return {
        _zoomVal = 0,
		_inventoryChangedListener = nil,
        _escapeBtn = nil,
        _pGamelayer = nil,

        -- two way binding
		Get_ZoomVal = function(self)
			return self._zoomVal
		end,
		Set_ZoomVal = function(self, val)
			print(val)
			self._zoomVal = val
            SetGameLayerZoom(self._zoomVal, self._pGamelayer)
		end,

		OnInitSettings = function(self, pGamelayer)
			print(dump(args))
            self._pGamelayer = pGamelayer
            self._zoomVal = GetGameLayerZoom(pGamelayer)
            self.Set_ZoomVal(self, self._zoomVal)
            OnPropertyChanged(self,"ZoomVal")

		end,

        OnInput = function(self)
            if GetButtonPress(self._escapeBtn) then
                PopGameFrameworkLayer()
                WfPushHUD()
            end
        end,

        OnXMLUILayerPush = function(self)
            self._escapeBtn = GetButtonBinding("settings")
            print(self._escapeBtn)
			self._initializeSettingsMenuListener = SubscribeGameFrameworkEvent("InitSettings", self, self.OnInitSettings)
            FireGameFrameworkEvent({vm=self, type="basic"}, "onSettingsMenuPushed")
        end,

		OnXMLUILayerPop = function(self)
			print("POP!")
            FreeButtonBinding(self._escapeBtn)
			UnsubscribeGameFrameworkEvent(self._initializeSettingsMenuListener)
		end

    }
end