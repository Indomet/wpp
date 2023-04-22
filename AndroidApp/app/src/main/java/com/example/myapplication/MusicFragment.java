package com.example.myapplication;

import android.os.Bundle;

import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import java.util.ArrayList;


public class MusicFragment extends Fragment {

    private RecyclerView recyclerView;
    private View rootView;
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        rootView = inflater.inflate(R.layout.fragment_music, container, false);
        recyclerView = rootView.findViewById(R.id.songLibraryView);


        //The song list database
        //TODO: replace with reading data from music list file
        ArrayList<Song> songsList = new ArrayList<>();
        songsList.add(new Song("Song 1", 180, 200, "", true));
        songsList.add(new Song("Song 2", 200, 250, "", true));
        songsList.add(new Song("Song 3", 170, 300, "", false));

        SongLibraryAdapter adapter = new SongLibraryAdapter(recyclerView.getContext());
        adapter.setSongsList(songsList);
        recyclerView.setAdapter(adapter);
        recyclerView.setLayoutManager(new LinearLayoutManager(recyclerView.getContext(), LinearLayoutManager.VERTICAL, false));
        //Linear displays a single line of items vertically


        return rootView;
    }
}