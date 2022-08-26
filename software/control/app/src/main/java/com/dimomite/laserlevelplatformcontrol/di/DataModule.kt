package com.dimomite.laserlevelplatformcontrol.di

import com.dimomite.laserlevelplatformcontrol.data.PlatformControl
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import retrofit2.Retrofit
import retrofit2.converter.moshi.MoshiConverterFactory
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object DataModule {

    @Provides
    @Singleton
    fun retrofit(): Retrofit =
        Retrofit.Builder()
            .baseUrl("http://192.168.4.1/api/")
            .addConverterFactory(MoshiConverterFactory.create())
            .build()

    @Provides
    @Singleton
    fun platformControl(retrofit: Retrofit): PlatformControl =
        retrofit.create(PlatformControl::class.java)

}
