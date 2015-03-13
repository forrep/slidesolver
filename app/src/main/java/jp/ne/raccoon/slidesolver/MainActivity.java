package jp.ne.raccoon.slidesolver;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import jp.ne.raccoon.slidesolver.Solver.Direction;
import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends Activity {
	private static final String vm;
	static {
		String vmVersion = System.getProperty("java.vm.version");
		if (vmVersion == null || vmVersion.startsWith("0.") || vmVersion.startsWith("1.")) {
			vm = "Dalvik";
		} else {
			vm = "ART";
		}
	}

	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        ((Button) findViewById(R.id.startCalc)).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				startCalc();
				final SlideSolverTask task = new SlideSolverTask();
				task.execute(new ExecutionType[]{ExecutionType.VM});
		        ((Button) findViewById(R.id.cancel)).setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						v.setEnabled(false);
						task.cancel(false);
					}
				});
			}
		});
        ((Button) findViewById(R.id.startCalcArrayed)).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				startCalc();
				final SlideSolverTask task = new SlideSolverTask();
				task.execute(new ExecutionType[]{ExecutionType.VM_ARRAYED});
		        ((Button) findViewById(R.id.cancel)).setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						v.setEnabled(false);
						task.cancel(false);
					}
				});
			}
		});
        ((Button) findViewById(R.id.startCalcNative)).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				startCalc();
				final SlideSolverTask task = new SlideSolverTask();
				task.execute(new ExecutionType[]{ExecutionType.NATIVE});
		        ((Button) findViewById(R.id.cancel)).setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						v.setEnabled(false);
						task.cancel(false);
					}
				});
			}
		});
    }
	
	private void startCalc() {
		((Button) findViewById(R.id.startCalc)).setEnabled(false);
		((Button) findViewById(R.id.startCalcArrayed)).setEnabled(false);
		((Button) findViewById(R.id.startCalcNative)).setEnabled(false);
		((Button) findViewById(R.id.cancel)).setEnabled(true);
	}
	
	private void finishCalc() {
		((Button) findViewById(R.id.startCalc)).setEnabled(true);
		((Button) findViewById(R.id.startCalcArrayed)).setEnabled(true);
		((Button) findViewById(R.id.startCalcNative)).setEnabled(true);
		((Button) findViewById(R.id.cancel)).setEnabled(false);
	}
	
	public static enum ExecutionType {
		VM, VM_ARRAYED, NATIVE
	}
	private class SlideSolverTask extends AsyncTask<ExecutionType, String, String> {
		@Override
		protected String doInBackground(ExecutionType... params) {
			// 下記のコメントを解除するとHeapGuardianが有効になる。
			// ヒープ領域を指定容量分まとめて確保した後に最後尾にいくつかオブジェクトを確保して、
			// 最初に確保した領域を解放することでヒープ領域の縮小を防止する。※ DalvikがCompactionを行わない事を前提に動作。
			// Dalvik環境ではこれだけで1～2割高速化するが、softLimitという制限の影響でそれ以上は高速化しない。
			// 何らかの最適化でうまく動作しない場合はfinally句の guardian.ack(); を有効にする必要があるかもしれない。
			// 
			// HeapGuardian guardian = new HeapGuardian(134217728); //128mb
			
			ExecutionType executionType = params.length > 0 && params[0] != null ? params[0] : ExecutionType.VM;
			String mode;
			if (executionType == ExecutionType.VM_ARRAYED) {
				mode = new StringBuilder().append(vm).append("(Arrayed)").toString();
			}
			else {
				mode = executionType == ExecutionType.NATIVE ? "Native" : vm;
			}
			BufferedReader reader = null;
			try {
				reader = new BufferedReader(new InputStreamReader(MainActivity.class.getResourceAsStream("problems.txt")));
				// 移動数制限
				String[] limitsString = reader.readLine().split(" ");
				// 問題数
				int count = Integer.parseInt(reader.readLine());
				Solver.MoveLimit moveLimit = new Solver.MoveLimit(Integer.parseInt(limitsString[0]), Integer.parseInt(limitsString[1]), Integer.parseInt(limitsString[2]), Integer.parseInt(limitsString[3]));
				int lineNumber = 0;
				int solved = 0;
				String line;
				long start = System.currentTimeMillis();
				publishProgress("Started");
				while ((line = reader.readLine()) != null) {
					if (isCancelled()) {
						break;
					}
					++lineNumber;
					Log.d("SlideSolverTask", new StringBuilder().append("#").append(lineNumber).toString());
					
					String[] splitedLine = line.split(",");
					int width = Integer.parseInt(splitedLine[0]);
					int height = Integer.parseInt(splitedLine[1]);
					String fieldString = splitedLine[2];
					
					boolean isSolved = false;
					String operations = "";
					int operationCount;
					if (executionType == ExecutionType.VM_ARRAYED) {
						SolverArrayed.FieldOperation answerField = SolverArrayed.solve(width, height, fieldString, false);
						isSolved = answerField.isSolved();
						operations = answerField.getOperations();
						operationCount = answerField.getOperationCount();
					}
					else {
						Solver.FieldOperation answerField = Solver.solve(width, height, fieldString, executionType == ExecutionType.NATIVE);
						isSolved = answerField.isSolved();
						operations = answerField.getOperations();
						operationCount = answerField.getOperationCount();
					}
					if (isSolved) {
						++solved;
						moveLimit.add(operations);
					}
					
					long elapsed = System.currentTimeMillis() - start;
					long remain = Math.max(elapsed * count / lineNumber - elapsed, 0);
					publishProgress(new StringBuilder().append("#").append(lineNumber).append("\n")
							.append("Mode: ").append(mode).append("\n")
							.append("Input: ").append(line).append("\n")
							.append("Moves: ").append(operations).append("\n")
							.append("Count: ").append(operationCount).append("\n")
							.append("Elapsed: ").append(elapsed / 1000).append(" sec\n")
							.append("Remain: ").append(remain / 1000).append(" sec\n")
							.append("Total: ").append((elapsed + remain) / 1000).append(" sec\n")
							.toString());
				}
				reader.close();

				return new StringBuilder().append("Finished!\n")
						.append("Mode: ").append(mode).append("\n")
						.append("Num: ").append(lineNumber).append("\n")
						.append("Solved: ").append(solved).append("\n")
						.append("Elapsed: ").append(((System.currentTimeMillis() - start) / 1000)).append(" sec\n")
						.append("Limits(L): ").append(moveLimit.moves[Direction.LEFT .code]).append("/").append(moveLimit.limits[Direction.LEFT .code]).append("\n")
						.append("Limits(R): ").append(moveLimit.moves[Direction.RIGHT.code]).append("/").append(moveLimit.limits[Direction.RIGHT.code]).append("\n")
						.append("Limits(U): ").append(moveLimit.moves[Direction.UP   .code]).append("/").append(moveLimit.limits[Direction.UP   .code]).append("\n")
						.append("Limits(D): ").append(moveLimit.moves[Direction.DOWN .code]).append("/").append(moveLimit.limits[Direction.DOWN .code]).append("\n")
						.toString();
			} catch (IOException e) {
				Log.d("SlideSolverTask", "doInBackground", e);
			}
			finally {
				if (reader != null) {
					try {
						reader.close();
					} catch (IOException e) {
					}
				}
//				guardian.ack();
			}
			return null;
		}
		@Override
		protected void onProgressUpdate(String... values) {
			((TextView) findViewById(R.id.progressText)).setText(values[0]);
		}
		@Override
		protected void onPostExecute(String result) {
			if (result != null) {
				((TextView) findViewById(R.id.progressText)).setText(result);
			}
			finishCalc();
		}
		@Override
		protected void onCancelled() {
			((TextView) findViewById(R.id.progressText)).setText("Canceled.");
			finishCalc();
		}
	}
}
