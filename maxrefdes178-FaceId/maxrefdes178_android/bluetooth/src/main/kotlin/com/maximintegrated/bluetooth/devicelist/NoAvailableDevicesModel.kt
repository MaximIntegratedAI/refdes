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

package com.maximintegrated.bluetooth.devicelist

import android.view.View
import android.widget.TextView
import com.airbnb.epoxy.EpoxyAttribute
import com.airbnb.epoxy.EpoxyHolder
import com.airbnb.epoxy.EpoxyModelClass
import com.airbnb.epoxy.EpoxyModelWithHolder
import com.google.android.material.button.MaterialButton
import com.maximintegrated.bluetooth.R
import com.maximintegrated.bluetooth.devicelist.NoAvailableDevicesModel.NoAvailableDevicesHolder

@EpoxyModelClass
abstract class NoAvailableDevicesModel : EpoxyModelWithHolder<NoAvailableDevicesHolder>() {

    override fun getDefaultLayout() = R.layout.model_no_available_devices_message

    @EpoxyAttribute(hash = false)
    var deviceClickListener: OnBluetoothDeviceClickListener? = null

    override fun bind(holder: NoAvailableDevicesHolder) {
        holder.demoButton.setOnClickListener {
            deviceClickListener?.onBluetoothDeviceClicked(null)
        }
    }

    class NoAvailableDevicesHolder : EpoxyHolder() {
        lateinit var textView: TextView
        lateinit var demoButton: MaterialButton

        override fun bindView(itemView: View) {
            textView = itemView.findViewById(R.id.message_text)
            demoButton = itemView.findViewById(R.id.demo_button)
        }
    }
}