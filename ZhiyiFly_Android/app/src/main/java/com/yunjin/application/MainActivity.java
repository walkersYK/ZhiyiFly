package com.yunjin.application;

import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.webkit.WebView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.MediaController;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

import androidx.appcompat.app.AppCompatActivity;
import com.yunjin.application.Struct.sReceiveRemote;
import com.yunjin.application.Socket.Client;
import com.yunjin.application.unit.JoystickView;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.Locale;

public class MainActivity extends AppCompatActivity
        implements JoystickView.OnJoystickMoveListener {

    private Client client;
    private sReceiveRemote controlData = new sReceiveRemote();
    private TextView tvStatus, logView;
    MediaController mMediaController;
    private EditText ed_port;
    private WebView webview1;
    private long lastSendTime = 0;
    private final LinkedList<String> logQueue = new LinkedList<>();
    private static final int MAX_LOG_ENTRIES = 10;

    // 监听器实现
    private final Client.DataListener dataListener = new Client.DataListener() {
        @Override
        public void onSend(String data) {
            appendLog("[SEND] " + data);
        }

        @Override
        public void onReceive(String result) {
            appendLog("[RECV] " + result);
        }

        @Override
        public void onFail(Exception e) {
            appendLog("[ERROR] " + e.getMessage());
        }
    };

    private final Client.ConnectListener connectListener = new Client.ConnectListener() {
        @Override
        public void onConnected() {
            client.send("#CLIENT_BEGAIN#");
            runOnUiThread(() -> {
                Toast.makeText(MainActivity.this,"连接成功",Toast.LENGTH_SHORT).show();
                tvStatus.setText("已连接");
                ((Button)findViewById(R.id.bt_connect)).setText("断开");
            });
        }

        @Override
        public void onDisconnected() {
            runOnUiThread(() -> {
                tvStatus.setText("未连接");
                ((Button)findViewById(R.id.bt_connect)).setText("连接");
            });
        }

        @Override
        public void onError(Exception e) {
            appendLog("[CONN ERR] " + e.getMessage());
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        if (getSupportActionBar() != null) {
            getSupportActionBar().hide();
        }

        // 初始化UI组件
        tvStatus = findViewById(R.id.tv_status);
        logView = findViewById(R.id.log_windows);
        Button btnConnect = findViewById(R.id.bt_connect);
        EditText edAddress = findViewById(R.id.ed_connect_address);
        EditText edPort = findViewById(R.id.ed_connect_port_code);

        // 设置默认连接参数
        edAddress.setText("192.168.4.1");
        edPort.setText("8088");
        // 视频初始化
        mMediaController = new MediaController(this);
        webview1 = (WebView) findViewById(R.id.webview1);
        setview();

        // 摇杆初始化
        JoystickView leftJoystick = findViewById(R.id.left_joystick);
        JoystickView rightJoystick = findViewById(R.id.right_joystick);
        leftJoystick.setListener(this);
        rightJoystick.setListener(this);

        // 连接按钮事件
        btnConnect.setOnClickListener(v -> {
            if (client != null && client.isConnected) {
                client.disconnect();
            } else {
                String ip = edAddress.getText().toString();
                int port = Integer.parseInt(edPort.getText().toString());
                client = new Client(dataListener, ip, port);
                client.connect(connectListener);
            }
        });


    }
    private void setview() {
        String url="http://192.168.8.13:8080/?action=stream";//视频链接
        webview1.loadUrl(url);//打开指定URL的html文件

    }

    @Override
    public void onValueChanged(int type, float xPercent, float yPercent) {
        if (client == null || !client.isConnected) return;

        // 节流控制（50ms发送间隔）
        if (System.currentTimeMillis() - lastSendTime < 50) return;
        lastSendTime = System.currentTimeMillis();

        // 更新控制数据（根据摇杆类型）
        switch(type) {
            case 0: // 左摇杆：Y轴油门（上下推）
                controlData.THR = (short)(-yPercent * 32767); // 注意Y轴方向
                break;
            case 1: // 右摇杆：X轴偏航（左右），Y轴俯仰（上下）
                controlData.YAW = (short)(xPercent * 32767);
                controlData.PIT = (short)(-yPercent * 32767);
                break;
        }

        // 打包并发送数据
        byte[] packet = packControlData();

        System.out.println(Arrays.toString(packet));

        client.send(packet);
    }

    private byte[] packControlData() {
        ByteBuffer buffer = ByteBuffer.allocate(8); // 4个short=8字节
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        buffer.putShort(controlData.THR);
        buffer.putShort(controlData.YAW);
        buffer.putShort(controlData.ROL);
        buffer.putShort(controlData.PIT);
        return buffer.array();
    }

    private void appendLog(String text) {
        runOnUiThread(() -> {
            logQueue.add(text);
            while (logQueue.size() > MAX_LOG_ENTRIES) {
                logQueue.removeFirst();
            }
            StringBuilder sb = new StringBuilder();
            for (String entry : logQueue) {
                sb.append(entry).append("\n");
            }
            logView.setText(sb.toString());
            // 自动滚动到底部
            int scrollAmount = logView.getLayout().getLineTop(logView.getLineCount()) - logView.getHeight();
            logView.scrollTo(0, Math.max(scrollAmount, 0));
        });
    }

    @Override
    protected void onDestroy() {
        if (client != null) client.disconnect();
        super.onDestroy();
    }
}