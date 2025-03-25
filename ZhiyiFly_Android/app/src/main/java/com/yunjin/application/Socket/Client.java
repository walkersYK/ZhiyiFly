package com.yunjin.application.Socket;

import android.util.Log;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.charset.StandardCharsets;

public class Client {
    public interface DataListener {
        void onSend(String data);

        void onReceive(String result);

        void onFail(Exception e);
    }
   public interface ConnectListener{
        void onConnected();
        void onDisconnected();
        void onError(Exception e);
   }
//    数据发送、接收和失败时的回调方法
    private final DataListener listener;
    private final String address; //服务器地址
    private final int port;  //端口号
    private Socket socket;  //Socket对象，用于网络通信
    public boolean isConnected=false;
//    连接成功、断开连接和连接错误时的回调方法
    private ConnectListener connectListener=null;
    private final Send sendThread;
//    构造函数，初始化监听器、地址、端口以及启动发送线程。
    public Client(DataListener listener, String address, int port) {
        this.address = address;
        this.port = port;
        this.listener = listener;
        sendThread=new Send();
        sendThread.start();
    }
    // 在 Client.java 中添加
    public void disconnect() {
        try {
            if (socket != null && !socket.isClosed()) {
                socket.close();
            }
            isConnected = false;
            connectListener.onDisconnected();
        } catch (IOException e) {
            connectListener.onError(e);
        }
    }
    //    设置要发送的数据，并通知发送线程进行发送。
    public void send(String data){
        if (sendThread!=null){
            sendThread.data=data;
            sendThread.flag=true;
        }
    }
    // 添加二进制发送方法
    public void send(byte[] data) {
        if (sendThread != null) {
            sendThread.dataBytes = data;
            sendThread.flag = true;
        }
    }
    //    尝试连接到指定地址和端口的服务器，并在连接成功后启动接收线程。
    public void connect(ConnectListener listener)
    {
        this.connectListener=listener;
        new Thread(() -> {
            try {
                socket=new Socket(address,port);
                listener.onConnected();
//                接收线程
                ReceiveThread thread=new ReceiveThread();
                thread.start();
                isConnected=true;
            } catch (IOException e) {
                e.printStackTrace();
                listener.onError(e);
                connectListener.onDisconnected();
                isConnected=false;
            }
        }).start();

    }
    class Send extends Thread{
       public String data;
       public byte[] dataBytes;
       public boolean flag=false;
        @Override
        public void run() {
            while (true) {
                if (flag) {
                    if (dataBytes != null) {
                        sendBinary(dataBytes);
                        dataBytes = null;
                    } else if (data != null) {
                        sendT(data);
                        data = null;
                    }
                    flag = false;
                }
                try { Thread.sleep(10); } catch (InterruptedException e) {}
            }
        }
    }
    // 使用DataOutputStream将数据写入Socket的输出流，并调用监听器的onSend方法。
    private void sendT(String data) {
                OutputStream outputStream = null;
                DataOutputStream dataOutputStream=null;
                try {
                    outputStream = socket.getOutputStream();
                   dataOutputStream=new DataOutputStream(new BufferedOutputStream(outputStream));
                   dataOutputStream.writeUTF(data);
                   dataOutputStream.flush();
                   listener.onSend(data);
                } catch (IOException e) {
                    e.printStackTrace();
                    listener.onFail(e);
                    connectListener.onDisconnected();
                    isConnected=false;
                }
    }
    // 新增二进制发送方法
    private void sendBinary(byte[] data) {
        try {
            OutputStream outputStream = socket.getOutputStream();
            outputStream.write(data);
            outputStream.flush();
            listener.onSend("[BIN] " + bytesToHex(data));
        } catch (IOException e) {
            listener.onFail(e);
        }
    }
    // 字节数组转十六进制字符串
    private String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02X ", b));
        }
        return sb.toString();
    }
    public void receive() {
        try {
            InputStream inputStream = socket.getInputStream();
            byte[] buffer = new byte[1024];
            int bytesRead;

            Log.d("开始","test");

            while ((bytesRead = inputStream.read(buffer)) != -1) {
                String result = new String(buffer, 0, bytesRead, StandardCharsets.UTF_8);
                Log.d("RawPacket", result);
                listener.onReceive(result);

                // 如果需要处理二进制数据可以添加：
                // String hexData = bytesToHex(Arrays.copyOf(buffer, bytesRead));
                // listener.onReceive("[BIN] " + hexData);
            }

            // 连接断开处理
            connectListener.onDisconnected();
            isConnected = false;
            socket.close();
        } catch (IOException e) {
            listener.onFail(e);
            connectListener.onDisconnected();
            isConnected = false;
        }
    }

    // 修改后的 ReceiveThread
    class ReceiveThread extends Thread {
        @Override
        public void run() {
            receive(); // 直接调用阻塞式读取
        }
    }
}
