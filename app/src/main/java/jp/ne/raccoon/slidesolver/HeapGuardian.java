package jp.ne.raccoon.slidesolver;

import android.annotation.SuppressLint;

public class HeapGuardian {
	private static Integer[] guardian = new Integer[100];
	
	@SuppressLint("UseValueOf")
	public HeapGuardian(int size) {
		int[] temp = new int[size / 4];
		for (int i = 0; i < guardian.length; ++i) {
			guardian[i] = new Integer(temp[Math.min(i, temp.length - 1)]);
		}
	}
	public void ack() {
		// do nothing
	}
}
