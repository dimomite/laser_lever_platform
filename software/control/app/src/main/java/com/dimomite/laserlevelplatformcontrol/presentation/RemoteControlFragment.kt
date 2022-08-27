package com.dimomite.laserlevelplatformcontrol.presentation

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import com.dimomite.laserlevelplatformcontrol.databinding.FragmentRemoteControlBinding
import dagger.hilt.android.AndroidEntryPoint

@AndroidEntryPoint
class RemoteControlFragment : Fragment() {

    private val vm: RemoteControlViewModel by viewModels()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val bind = FragmentRemoteControlBinding.inflate(inflater, container, false)
        bind.lifecycleOwner = this
        bind.model = vm
        return bind.root
    }

}
