using CircularBuffer;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace BresserGoTo_ReverseEngineer
{
    public class SerialReverseEngineerDataModel : INotifyPropertyChanged
    {
        public SerialReverseEngineerDataModel()
        {
            string[] serialNames = SerialPort.GetPortNames();

            foreach(string serialName in serialNames)
            {
                SerialPortNames.Add(serialName);
            }

            SerialPort.DataReceived += SerialDataReceived;

            Time_Hours = DateTime.Now.Hour;
            Time_Minutes = DateTime.Now.Minute;
            Time_Seconds = DateTime.Now.Second;

            Date_Years = DateTime.Now.Year;
            Date_Months = DateTime.Now.Month;
            Date_Days = DateTime.Now.Day;
        }

        private SerialPort SerialPort { get; set; } = new SerialPort();

        public ObservableCollection<string> SerialPortNames { get; internal set; } = new ObservableCollection<string>();

        private ICommand _connectSerialCommand;
        private ICommand _gotoCommand;
        private ICommand _syncCommand;
        private ICommand _parkCommand;
        private ICommand _disconnectCommand;

        private ICommand _eastCommand;
        private ICommand _westCommand;
        private ICommand _northCommand;
        private ICommand _southCommand;

        private ICommand _fuzzCommand;

        private ICommand _stopCommand;
        private ICommand _setTimeDateCommand;
        private ICommand _setLocationCommand;
        private ICommand _getLocationCommand;

        private ICommand _convertFloatCommand;

        public ObservableCollection<RawSerialMessage> ReportMessages { get; internal set; } = new ObservableCollection<RawSerialMessage>();

        public ICommand ConnectSerialCommand
        {
            get
            {
                if (_connectSerialCommand == null)
                {
                    _connectSerialCommand = new RelayCommand(ConnectSerial);
                }
                return _connectSerialCommand;
            }
        }


        public ICommand DisconnectCommand
        {
            get
            {
                if (_disconnectCommand == null)
                {
                    _disconnectCommand = new RelayCommand(Disconnect);
                }
                return _disconnectCommand;
            }
        }

        public ICommand GoToCommand
        {
            get
            {
                if (_gotoCommand == null)
                {
                    _gotoCommand = new RelayCommand(GoToPosition);
                }
                return _gotoCommand;
            }
        }

        public ICommand SyncCommand
        {
            get
            {
                if (_syncCommand == null)
                {
                    _syncCommand = new RelayCommand(SyncPosition);
                }
                return _syncCommand;
            }
        }

        public ICommand ParkCommand
        {
            get
            {
                if (_parkCommand == null)
                {
                    _parkCommand = new RelayCommand(ParkPosition);
                }
                return _parkCommand;
            }
        }

        public ICommand FuzzCommand
        {
            get
            {
                if (_fuzzCommand == null)
                {
                    _fuzzCommand = new RelayCommand(Fuzz);
                }
                return _fuzzCommand;
            }
        }

        public ICommand StopCommand
        {
            get
            {
                if (_stopCommand == null)
                {
                    _stopCommand = new RelayCommand(StopMotion);
                }
                return _stopCommand;
            }
        }

        public ICommand SetTimeDateCommand
        {
            get
            {
                if (_setTimeDateCommand == null)
                {
                    _setTimeDateCommand = new RelayCommand(SetTimeDate);
                }
                return _setTimeDateCommand;
            }
        }

        public ICommand SetLocationCommand
        {
            get
            {
                if (_setLocationCommand == null)
                {
                    _setLocationCommand = new RelayCommand(SetLocation);
                }
                return _setLocationCommand;
            }
        }

        public ICommand GetLocationCommand
        {
            get
            {
                if (_getLocationCommand == null)
                {
                    _getLocationCommand = new RelayCommand(GetLocation);
                }
                return _getLocationCommand;
            }
        }

        public ICommand EastCommand
        {
            get
            {
                if (_eastCommand == null)
                {
                    _eastCommand = new RelayCommand(GoEast);
                }
                return _eastCommand;
            }
        }

        public ICommand WestCommand
        {
            get
            {
                if (_westCommand == null)
                {
                    _westCommand = new RelayCommand(GoWest);
                }
                return _westCommand;
            }
        }

        public ICommand NorthCommand
        {
            get
            {
                if (_northCommand == null)
                {
                    _northCommand = new RelayCommand(GoNorth);
                }
                return _northCommand;
            }
        }

        public ICommand SouthCommand
        {
            get
            {
                if (_southCommand == null)
                {
                    _southCommand = new RelayCommand(GoSouth);
                }
                return _southCommand;
            }
        }

        public ICommand FloatConvertCommand
        {
            get
            {
                if (_convertFloatCommand == null)
                {
                    _convertFloatCommand = new RelayCommand(ConvertFloat);
                }
                return _convertFloatCommand;
            }
        }

        public void ConnectSerial(object obj)
        {
            if (obj != null)
            {
                if (obj is string)
                {
                    string portName = obj as string;

                    Debug.WriteLine($"using {portName} ...");
                   try
                   {
                       if(SerialPort.IsOpen)
                       {
                           SerialPort.Close();
                       }
                       else
                       {
                           SerialPort.PortName = portName;
                           SerialPort.BaudRate = 9600;
                           SerialPort.Parity = Parity.None;
                           SerialPort.DataBits = 8;
                           SerialPort.StopBits = StopBits.One;
                           SerialPort.Handshake = Handshake.None;

                           SerialPort.Open();

                            ReportMessages.Clear();
                       }
                   }
                   catch(Exception e)
                   {
                       Debug.WriteLine($"failed to establish serial connection on {portName}: {e}");
                       return;
                   }
                }
            }
        }

        public int RA_Hour { get; set; }   = 0;
        public int RA_Minute { get; set; } = 0;
        public int RA_Second { get; set; } = 0;

        public int DEC_Degrees { get; set; } = 0;
        public int DEC_Minute { get; set; } = 0;
        public int DEC_Second { get; set; } = 0;

        public int Time_Hours { get; set; } = 0;
        public int Time_Minutes { get; set; } = 0;
        public int Time_Seconds { get; set; } = 0;

        public int Date_Days { get; set; } = 0;
        public int Date_Months { get; set; } = 0;
        public int Date_Years { get; set; } = 0;

        public int LatitudeDegrees { get; set; } = 0;
        public int LatitudeMinutes { get; set; } = 0;
        public int LatitudeSeconds { get; set; } = 0;

        public int LongitudeDegrees { get; set; } = 0;
        public int LongitudeMinutes { get; set; } = 0;
        public int LongitudeSeconds { get; set; } = 0;

        public byte CID { get; set; } = 0x00;
        public byte B1 { get; set; } = 0x00;
        public byte B2 { get; set; } = 0x00;
        public byte B3 { get; set; } = 0x00;
        public byte B4 { get; set; } = 0x00;
        public byte B5 { get; set; } = 0x00;
        public byte B6 { get; set; } = 0x00;
        public byte B7 { get; set; } = 0x00;
        public byte B8 { get; set; } = 0x00;

        public float FLoatConvert { get; set; } = 0.24f;

        public string FloatByteResult { get; set; } = "00 00 00 00";

        public string DoubleByteResult { get; set; } = "00 00 00 00 00 00 00 00";

        public void Disconnect(object obj)
        {
            RawSerialMessage disconnectMessage = RawSerialMessage.GetDisconnectMessage();
            ReportMessages.Add(disconnectMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(disconnectMessage.MessageBuffer, 0, disconnectMessage.MessageBuffer.Length);
            }

            Debug.WriteLine($"Message : {disconnectMessage}");
        }

        public void GoEast(object obj)
        {
            RawSerialMessage goEastMessage = RawSerialMessage.GetGetGoDirectionCommandMessage(RawSerialMessage.MOVE_EAST_COMMAND);
            ReportMessages.Add(goEastMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(goEastMessage.MessageBuffer, 0, goEastMessage.MessageBuffer.Length);
            }
        }

        public void GoWest(object obj)
        {
            RawSerialMessage goWestMessage = RawSerialMessage.GetGetGoDirectionCommandMessage(RawSerialMessage.MOVE_WEST_COMMAND);
            ReportMessages.Add(goWestMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(goWestMessage.MessageBuffer, 0, goWestMessage.MessageBuffer.Length);
            }
        }

        public void GoNorth(object obj)
        {
            RawSerialMessage goNorthMessage = RawSerialMessage.GetGetGoDirectionCommandMessage(RawSerialMessage.MOVE_NORTH_COMMAND);
            ReportMessages.Add(goNorthMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(goNorthMessage.MessageBuffer, 0, goNorthMessage.MessageBuffer.Length);
            }
        }

        public void GoSouth(object obj)
        {
            RawSerialMessage goSouthMessage = RawSerialMessage.GetGetGoDirectionCommandMessage(RawSerialMessage.MOVE_SOUTH_COMMAND);
            ReportMessages.Add(goSouthMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(goSouthMessage.MessageBuffer, 0, goSouthMessage.MessageBuffer.Length);
            }
        }

        public void GoToPosition(object obj)
        {
            RightAscension ra = new RightAscension() { Hours = RA_Hour, Minutes = RA_Minute, Seconds = RA_Second };
            Declination dec = new Declination() { Degrees = DEC_Degrees, Minutes = DEC_Minute, Seconds = DEC_Second };

            RawSerialMessage gotoMessage= RawSerialMessage.GetSetPointingLocationCommandMessage(ra, dec);
            ReportMessages.Add(gotoMessage);

            if(SerialPort.IsOpen)
            {
                SerialPort.Write(gotoMessage.MessageBuffer, 0, gotoMessage.MessageBuffer.Length);
            } 

            Debug.WriteLine($"Message : {gotoMessage}");
        }

        public void SyncPosition(object obj)
        {
            RightAscension ra = new RightAscension() { Hours = RA_Hour, Minutes = RA_Minute, Seconds = RA_Second };
            Declination dec = new Declination() { Degrees = DEC_Degrees, Minutes = DEC_Minute, Seconds = DEC_Second };

            RawSerialMessage syncMessage = RawSerialMessage.GetSyncPointingLocationCommandMessage(ra, dec);
            ReportMessages.Add(syncMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(syncMessage.MessageBuffer, 0, syncMessage.MessageBuffer.Length);
            }

            Debug.WriteLine($"Message : {syncMessage}");
        }

        public void ParkPosition(object obj)
        {
            RawSerialMessage parkMessage = RawSerialMessage.GetParkCommandMessage();
            ReportMessages.Add(parkMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(parkMessage.MessageBuffer, 0, parkMessage.MessageBuffer.Length);
            }
        }

        public void StopMotion(object obj)
        {
            RawSerialMessage haltMessage = RawSerialMessage.GetStopMotionCommandMessage();
            ReportMessages.Add(haltMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(haltMessage.MessageBuffer, 0, haltMessage.MessageBuffer.Length);
            }
        }

        public void SetTimeDate(object obj)
        {
            RawSerialMessage setTimeDateMessage = RawSerialMessage.GetSetTimeDateCommandMessage((byte)Time_Hours, (byte)Time_Minutes, (byte)Time_Seconds, (byte)Date_Days, (byte)Date_Months, (short)Date_Years);
            ReportMessages.Add(setTimeDateMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(setTimeDateMessage.MessageBuffer, 0, setTimeDateMessage.MessageBuffer.Length);
            }

            Debug.WriteLine($"Message : {setTimeDateMessage}");
        }

        public void SetLocation(object obj)
        {
            RawSerialMessage setSiteLocationMessage = RawSerialMessage.GetSetLocationCommandMessage((byte)LatitudeDegrees, (byte)LatitudeMinutes, (byte) LatitudeSeconds,(byte)LongitudeDegrees,(byte)LongitudeMinutes,(byte)LongitudeSeconds);
            ReportMessages.Add(setSiteLocationMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(setSiteLocationMessage.MessageBuffer, 0, setSiteLocationMessage.MessageBuffer.Length);
            }

            Debug.WriteLine($"Message : {setSiteLocationMessage}");
        }

        public void GetLocation(object obj)
        {
            RawSerialMessage setSiteLocationMessage = RawSerialMessage.GetGetLocationCommandMessage();
            ReportMessages.Add(setSiteLocationMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(setSiteLocationMessage.MessageBuffer, 0, setSiteLocationMessage.MessageBuffer.Length);
            }

            Debug.WriteLine($"Message : {setSiteLocationMessage}");
        }

        public void Fuzz(object obj)
        {
            RawSerialMessage unknownMessage = RawSerialMessage.GetFuzzCommandMessage(CID, B1,B2,B3,B4,B5,B6,B7,B8);
            ReportMessages.Add(unknownMessage);

            if (SerialPort.IsOpen)
            {
                SerialPort.Write(unknownMessage.MessageBuffer, 0, unknownMessage.MessageBuffer.Length);
            }
        }

        public void ConvertFloat(object obj)
        {
            byte[] bytes = BitConverter.GetBytes(FLoatConvert);

            double doubleValue = FLoatConvert;

            byte[] doubleBytes = BitConverter.GetBytes(doubleValue);

            FloatByteResult = $"{bytes[0]} {bytes[1]} {bytes[2]} {bytes[3]}";

            DoubleByteResult = $"{doubleBytes[0]} {doubleBytes[1]} {doubleBytes[2]} {doubleBytes[3]} {doubleBytes[4]} {doubleBytes[5]} {doubleBytes[6]} {doubleBytes[7]}";

            OnPropertyChanged("FloatByteResult");
            OnPropertyChanged("DoubleByteResult");
        }

        private List<RawSerialMessage> RawMessageBuffer { get; } = new List<RawSerialMessage>();
        private byte[] SerialBuffer { get; } = new byte[1024];
        private int BufferReadLength { get; set; } = 0;

        private CircularBuffer<byte> ReadBuffer { get; } = new CircularBuffer<byte>(1024);

        void SerialDataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            int len = 0;

            while (SerialPort.BytesToRead > 0)
            {
                len = SerialPort.BytesToRead;

                for(int i=0;i<len;i++)
                {
                    byte read = (byte)SerialPort.ReadByte();
                    ReadBuffer.PushBack(read);
                }
            }

            TryParseMessage();

            if (RawMessageBuffer.Count > 0)
            {
                foreach (RawSerialMessage message in RawMessageBuffer)
                {
                    Application.Current.Dispatcher.Invoke(() => {

                        ReportMessages.Add(message);

                        if(message.CID == RawSerialMessage.REPORT_POINTING_COORDINATES_RESPONSE)
                        {
                            //Debug.WriteLine($"Pointing Message : {message}");
                        }

                        if (message.CID == RawSerialMessage.REPORT_SITE_LOCATION_RESPONSE)
                        {
                            Debug.WriteLine($"Get Location Message : {message}");

                            LatitudeDegrees = message.Latitude.Degrees;
                            LatitudeMinutes = message.Latitude.Minutes;
                            LatitudeSeconds = message.Latitude.Seconds;

                            LongitudeDegrees = message.Longitude.Degrees;
                            LongitudeMinutes = message.Longitude.Minutes;
                            LongitudeSeconds = message.Longitude.Seconds;

                            OnPropertyChanged("LatitudeDegrees");
                            OnPropertyChanged("LatitudeMinutes");
                            OnPropertyChanged("LatitudeSeconds");

                            OnPropertyChanged("LongitudeDegrees");
                            OnPropertyChanged("LongitudeMinutes");
                            OnPropertyChanged("LongitudeSeconds");
                        }
                    });

                    //Debug.WriteLine($"Message : {message}");
                }

                RawMessageBuffer.Clear();
            }
        }

        private void TryParseMessage()
        {
            if(ReadBuffer.Size > RawSerialMessage.MESSAGE_FRAME_LENGTH)
            {
                byte[] buffer = ReadBuffer.ToArray();

                List<int> headerPositions = FindByteSequence(RawSerialMessage.HEADER,buffer).ToList();

                foreach (int position in headerPositions)
                {
                    //Debug.WriteLine($"header at : {position}");
                    if (position>0)
                    {
                        for(int i = 0;i<position;i++)
                        {
                            //drop incomplete/previous message in front if any...
                            ReadBuffer.PopFront();
                        }
                    }

                    buffer = ReadBuffer.ToArray();

                    if (buffer.Length > (position + RawSerialMessage.MESSAGE_FRAME_LENGTH))
                    {
                        RawSerialMessage message = new RawSerialMessage();

                        Buffer.BlockCopy(buffer, position, message.MessageBuffer, 0, RawSerialMessage.MESSAGE_FRAME_LENGTH);

                        message.UpdateValues();

                        RawMessageBuffer.Add(message);
                    }
                }
            }
        }
        
        private IEnumerable<int> FindByteSequence(byte[] search, byte[] buffer)
        {
            for (int i = 0; i < buffer.Length; i++)
            {
                if (buffer.Skip(i).Take(search.Length).SequenceEqual(search))
                {
                    yield return i;
                }
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        private void OnPropertyChanged(string property)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(property));
            }
        }
    }
}
