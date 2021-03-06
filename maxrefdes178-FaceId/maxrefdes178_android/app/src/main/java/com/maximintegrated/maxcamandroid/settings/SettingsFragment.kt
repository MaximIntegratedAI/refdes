/*
 * Copyright (C) Maxim Integrated Products, Inc., All rights Reserved.
 *
 * This software is protected by copyright laws of the United States and
 * of foreign countries. This material may also be protected by patent laws
 * and technology transfer regulations of the United States and of foreign
 * countries. This software is furnished under a license agreement and/or a
 * nondisclosure agreement and may only be used or reproduced in accordance
 * with the terms of those agreements. Dissemination of this information to
 * any party or parties not specified in the license agreement and/or
 * nondisclosure agreement is expressly prohibited.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 */

package com.maximintegrated.maxcamandroid.settings

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.Spinner
import androidx.appcompat.app.AlertDialog
import androidx.fragment.app.Fragment
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.ViewModelProviders
import com.maximintegrated.communication.MaxCamViewModel
import com.maximintegrated.maxcamandroid.MainViewModel
import com.maximintegrated.maxcamandroid.R
import com.maximintegrated.maxcamandroid.blePacket.ble_command_e
import com.maximintegrated.maxcamandroid.blePacket.ble_command_packet_t
import com.maximintegrated.maxcamandroid.blePacket.camera_clock_e
import com.maximintegrated.maxcamandroid.blePacket.debugger_select_e
import com.maximintegrated.maxcamandroid.databinding.FragmentSettingsBinding
import com.maximintegrated.maxcamandroid.utils.SettingsItemListener
import com.maximintegrated.maxcamandroid.utils.sendDefaultEmbeddings
import com.maximintegrated.maxcamandroid.utils.setup
import com.maximintegrated.maxcamandroid.utils.startScannerActivity
import kotlinx.android.synthetic.main.fragment_demo.*

class SettingsFragment : Fragment() {

    companion object {
        fun newInstance() = SettingsFragment()
    }

    private lateinit var mainViewModel: MainViewModel
    private lateinit var maxCamViewModel: MaxCamViewModel

    private var _binding: FragmentSettingsBinding? = null
    private val binding get() = _binding!!

    private var settingsItemListener: SettingsItemListener = object : SettingsItemListener {
        override fun onEnableButtonClick(itemName: String, command: ble_command_e) {
            maxCamViewModel.sendData(ble_command_packet_t.from(command).toByteArray())
        }

        override fun onDisableButtonClick(itemName: String, command: ble_command_e) {
            maxCamViewModel.sendData(ble_command_packet_t.from(command).toByteArray())
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        _binding = FragmentSettingsBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        mainViewModel =
            ViewModelProvider(
                requireActivity(),
                ViewModelProvider.AndroidViewModelFactory(requireActivity().application)
            ).get(MainViewModel::class.java)

        maxCamViewModel = ViewModelProviders.of(requireActivity()).get(MaxCamViewModel::class.java)

        binding.max78000VideoCnnItem.setup(
            "FaceID",
            ble_command_e.BLE_COMMAND_ENABLE_MAX78000_VIDEO_CNN_CMD,
            ble_command_e.BLE_COMMAND_DISABLE_MAX78000_VIDEO_CNN_CMD,
            settingsItemListener
        )

        binding.max78000VideoVflipItem.setup(
            "Vertical Flip",
            ble_command_e.BLE_COMMAND_ENABLE_MAX78000_VIDEO_VFLIP_CMD,
            ble_command_e.BLE_COMMAND_DISABLE_MAX78000_VIDEO_VFLIP_CMD,
            settingsItemListener
        )

        binding.max78000VideoFlashLedItem.setup(
            "Flash LED",
            ble_command_e.BLE_COMMAND_ENABLE_MAX78000_VIDEO_FLASH_LED_CMD,
            ble_command_e.BLE_COMMAND_DISABLE_MAX78000_VIDEO_FLASH_LED_CMD,
            settingsItemListener
        )

        binding.lcdSetting.setup(
            "LCD",
            ble_command_e.BLE_COMMAND_ENABLE_LCD_CMD,
            ble_command_e.BLE_COMMAND_DISABLE_LCD_CMD,
            settingsItemListener
        )

        binding.lcdStats.setup(
            "Statistics",
            ble_command_e.BLE_COMMAND_ENABLE_LCD_STATISCTICS_CMD,
            ble_command_e.BLE_COMMAND_DISABLE_LCD_STATISCTICS_CMD,
            settingsItemListener
        )

        binding.lcdProbability.setup(
            "Probability",
            ble_command_e.BLE_COMMAND_ENABLE_LCD_PROBABILITY_CMD,
            ble_command_e.BLE_COMMAND_DISABLE_LCD_PROBABILITY_CMD,
            settingsItemListener
        )

        binding.inactivityEnableItem.setup(
            "Inactivity Timer",
            ble_command_e.BLE_COMMAND_ENABLE_INACTIVITY_CMD,
            ble_command_e.BLE_COMMAND_DISABLE_INACTIVITY_CMD,
            settingsItemListener
        )

        binding.max78000VideoEnableItem.setup(
            "Video",
            ble_command_e.BLE_COMMAND_ENABLE_MAX78000_VIDEO_CMD,
            ble_command_e.BLE_COMMAND_DISABLE_MAX78000_VIDEO_CMD,
            settingsItemListener
        )

        binding.max78000AudioEnableItem.setup(
            "Audio",
            ble_command_e.BLE_COMMAND_ENABLE_MAX78000_AUDIO_CMD,
            ble_command_e.BLE_COMMAND_DISABLE_MAX78000_AUDIO_CMD,
            settingsItemListener
        )

        setupCameraClockAdapter(
            binding.cameraClockSelectSpinner,
            camera_clock_e.values().toList().slice(0 until camera_clock_e.values().size - 1)
        )
        binding.sendCameraClockButton.setOnClickListener {
            maxCamViewModel.sendData(
                ble_command_packet_t.from(
                    ble_command_e.BLE_COMMAND_MAX78000_VIDEO_CAMERA_CLOCK_CMD,
                    (binding.cameraClockSelectSpinner.selectedItem as camera_clock_e).toByteArray()
                ).toByteArray()
            )
        }

        setupDebuggerSelectAdapter(
            binding.debuggerSelectSpinner,
            debugger_select_e.values().toList().slice(0 until debugger_select_e.values().size - 1)
        )
        binding.sendDebuggerButton.setOnClickListener {
            maxCamViewModel.sendData(
                ble_command_packet_t.from(
                    ble_command_e.BLE_COMMAND_SET_DEBUGGER_CMD,
                    (binding.debuggerSelectSpinner.selectedItem as debugger_select_e).toByteArray()
                ).toByteArray()
            )
        }

        binding.shutdownButton.setOnClickListener {
            val alert = AlertDialog.Builder(requireContext())
            alert.setMessage(getString(com.maximintegrated.maxcamandroid.R.string.are_you_sure_to_shut_device_down))
            alert.setPositiveButton(getString(com.maximintegrated.maxcamandroid.R.string.yes)) { dialog, _ ->
                dialog.dismiss()
                maxCamViewModel.sendData(
                    ble_command_packet_t.from(ble_command_e.BLE_COMMAND_SHUT_DOWN_DEVICE_CMD)
                        .toByteArray()
                )
                startScannerActivity(it.context)
            }
            alert.setNegativeButton(getString(com.maximintegrated.maxcamandroid.R.string.cancel)) { dialog, _ ->
                dialog.dismiss()
            }
            alert.show()
        }

        binding.restartButton.setOnClickListener {
            val alert = AlertDialog.Builder(requireContext())
            alert.setMessage(getString(com.maximintegrated.maxcamandroid.R.string.are_you_sure_to_restart_device))
            alert.setPositiveButton(getString(com.maximintegrated.maxcamandroid.R.string.yes)) { dialog, _ ->
                dialog.dismiss()
                maxCamViewModel.sendData(
                    ble_command_packet_t.from(ble_command_e.BLE_COMMAND_RESTART_DEVICE_CMD)
                        .toByteArray()
                )
                startScannerActivity(it.context)
            }
            alert.setNegativeButton(getString(com.maximintegrated.maxcamandroid.R.string.cancel)) { dialog, _ ->
                dialog.dismiss()
            }
            alert.show()
        }
        binding.sendDefaultButton.setOnClickListener {
            sendDefaultEmbeddings(maxCamViewModel, mainViewModel, requireContext())
        }
    }

    private fun setupDebuggerSelectAdapter(spinner: Spinner, list: List<debugger_select_e>?) {
        ArrayAdapter(
            requireContext(),
            android.R.layout.simple_spinner_item,
            list ?: emptyList()
        ).also { adapter ->
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
            spinner.adapter = adapter
        }
    }

    private fun setupCameraClockAdapter(spinner: Spinner, list: List<camera_clock_e>?) {
        ArrayAdapter(
            requireContext(),
            android.R.layout.simple_spinner_item,
            list ?: emptyList()
        ).also { adapter ->
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
            spinner.adapter = adapter
        }
    }
}
