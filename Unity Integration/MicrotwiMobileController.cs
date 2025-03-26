using System;
using System.Collections;
using System.IO.Ports;
using UnityEngine;
using UnityEngine.InputSystem;

public class MicrotwiMobileController : MonoBehaviour
{
    public static MicrotwiMobileController Instance { private set; get; }

    public SerialPort serialPort;
    public string output;
    public Gamepad microtwiGamepad;
    public bool initialized;

    private int currentVibrationPriority = 0;
    private int currentLEDPriority = 0;

    private void Awake()
    {
        Instance = this;
    }

    void Start()
    {

#if !UNITY_ANDROID || UNITY_EDITOR

        // Use the existing serial communication method for Windows
        FindAndConnectToSerialDevice();

#endif

    }

    void Update()
    {
        if (!initialized) return;

#if !UNITY_ANDROID || UNITY_EDITOR

        if (serialPort != null && serialPort.IsOpen)
        {
            try
            {
                // Read data from ESP32
                if (serialPort.BytesToRead > 0)
                {
                    output = serialPort.ReadLine();
                    // Debug.Log("Data Received: " + output);
                }
            }
            catch (Exception)
            {
                serialPort = null;
                enabled = false;
                return;
                // Debug.LogError("Serial Read/Write Error: " + ex.Message);
            }
        }

#endif

    }

    void FindAndConnectToSerialDevice(int baudRate = 115200)
    {
        initialized = false;
        string[] availablePorts = SerialPort.GetPortNames();
        Debug.Log("Available Ports: " + string.Join(", ", availablePorts));

        foreach (string portName in availablePorts)
        {
            try
            {
                SerialPort testPort = new SerialPort(portName, baudRate) { DtrEnable = true }; // Temporary port for testing
                testPort.Open();
                testPort.ReadTimeout = 500;

                DateTime timeout = DateTime.Now.AddSeconds(3);
                while (DateTime.Now < timeout)
                {
                    if (testPort.BytesToRead > 0)
                    {
                        string data = testPort.ReadLine();
                        if (data.StartsWith("MOVE_"))
                        {
                            Debug.Log("Device identified on " + portName);
                            testPort.Close(); // Close the test port

                            // Assign serialPort and keep it open
                            serialPort = new SerialPort(portName, baudRate) { DtrEnable = true };
                            serialPort.Open();
                            microtwiGamepad = InputSystem.AddDevice<Gamepad>();
                            initialized = true;

                            SettingsMenu.Instance.SetUpTheMicrotwiController(true);

                            return;
                        }
                    }
                }

                testPort.Close(); // Close the test port if no match found
            }
            catch (Exception)
            {
                // Debug.Log($"Failed to communicate on {portName}: {ex.Message}");
            }
        }

        Debug.Log("No compatible device found!");
        enabled = false;
        SettingsMenu.Instance.SetUpTheMicrotwiController(false);
    }

    [ContextMenu("BLINK START")]
    public void ShowLEDOn()
    {
        SendData("LED_ON");
    }

    [ContextMenu("BLINK OFF")]
    public void ShowLEDOff()
    {
        SendData("LED_OFF");
    }

    [ContextMenu("VIBRATE ON")]
    public void TurnVibrationOn()
    {
        SendData("VIBRATE_ON");
    }

    [ContextMenu("VIBRATE OFF")]
    public void TurnVibrationOff()
    {
        SendData("VIBRATE_OFF");
    }

    [ContextMenu("DECREASE FADE")]
    public void DecreaseFade()
    {
        SendData("DECREASE_FADE");
    }

    [ContextMenu("INCREASE FADE")]
    public void IncreaseFade()
    {
        SendData("INCREASE_FADE");
    }

    [ContextMenu("RESET FADE")]
    public void ResetFade()
    {
        SendData("RESET_FADE");
    }

    [ContextMenu("USE LED")]
    public void UseLED()
    {
        SendData("USE_LED");
    }

    [ContextMenu("DONT USE LED")]
    public void DontUseLED()
    {
        SendData("DONT_USE_LED");
    }

    public IEnumerator BlinkLEDs(float seconds, int iterations, int priority)
    {
        if (currentLEDPriority > priority) yield break;

        if (iterations < 1 || iterations > 10) iterations = 1;

        currentLEDPriority = priority;
        for (int i = 0; i < iterations; i++)
        {
            ShowLEDOn();
            yield return new WaitForSecondsRealtime(seconds + .1f);
            ShowLEDOff();
        }

        currentLEDPriority = 0;
    }

    public IEnumerator Vibrate(float seconds, int iterations, int priority)
    {
        if (currentVibrationPriority > priority) yield break;

        if (iterations < 1 || iterations > 10) iterations = 1;

        currentVibrationPriority = priority;
        for (int i = 0; i < iterations; i++)
        {
            TurnVibrationOn();
            yield return new WaitForSecondsRealtime(seconds + .1f);
            TurnVibrationOff();
        }

        currentVibrationPriority = 0;
    }


    private void SendData(string message)
    {

#if !UNITY_ANDROID || UNITY_EDITOR

        if (serialPort != null && serialPort.IsOpen)
        {
            serialPort.WriteLine(message);
        }

#endif

    }

    private void OnDisable()
    {
        SettingsMenu.Instance.SetUpTheMicrotwiController(false);

        if (serialPort != null && serialPort.IsOpen)
        {
            serialPort.Close();
        }
    }

    void OnApplicationQuit()
    {

#if !UNITY_ANDROID || UNITY_EDITOR

        if (serialPort != null && serialPort.IsOpen)
        {
            serialPort.Close();
        }

#endif

    }
}
