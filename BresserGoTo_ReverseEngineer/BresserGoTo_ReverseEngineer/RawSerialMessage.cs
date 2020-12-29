using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BresserGoTo_ReverseEngineer
{
    public class RightAscension
    {
        public int Hours { get; set; } = 0;
        public int Minutes { get; set; } = 0;
        public int Seconds { get; set; } = 0;

        public static RightAscension ConvertFrom(float decimalValue)
        {
            Debug.WriteLine($"Decimal RA {decimalValue}");

            int hours = (int)Math.Truncate(decimalValue);
            int minutes = (int)Math.Truncate((decimalValue - hours) * 60);
            int seconds = (int)Math.Truncate((3600 * (decimalValue - hours) - 60 * minutes));

            return new RightAscension() { Hours=hours,Minutes=minutes,Seconds=seconds };
        }

        public float DecimalValue
        {
            get
            {
                float decimal_value = Hours + ((float)Minutes / 60.0f) + ((float)Seconds / 3600.0f);
                return decimal_value;
            }
        }

        public override string ToString()
        {
            return $"{Hours}h {Minutes}' {Seconds}'' ({DecimalValue})";
        }
    }

    public class Declination
    {
        //public float DecimalValue { get; }
        public int Degrees { get; set; } = 0;
        public int Minutes { get; set; } = 0;
        public int Seconds { get; set; } = 0;

        public static Declination ConvertFrom(float decimalValue)
        {
            Debug.WriteLine($"Decimal Declination {decimalValue}");

            int degrees = (int)Math.Truncate(decimalValue);
            int minutes = (int)Math.Truncate((decimalValue - degrees) * 60);
            int seconds = (int)Math.Truncate((3600 * (decimalValue - degrees) - 60 * minutes));

            return new Declination() { Degrees = degrees, Minutes = minutes, Seconds = seconds };
        }

        public float DecimalValue
        {
            get
            {
                float decimal_value = Degrees + (Minutes / 60.0f) + (Seconds / 3600.0f);
                return decimal_value;
            }
        }

        public override string ToString()
        {
            return $"{Degrees}° {Minutes}' {Seconds}'' ({DecimalValue})";
        }
    }

    public class RawSerialMessage
    {
        public byte[] MessageBuffer { get; set; } = new byte[13];

        public static byte[] HEADER { get; } = { 0x55, 0xaa, 0x01, 0x09 };

        public static byte REPORT_POINTING_COORDINATES_RESPONSE { get; } = 0xff;

        public static byte REPORT_SITE_LOCATION_RESPONSE { get; } = 0xfe;

        public static byte DISCONNECT_COMMAND { get; } = 0x22;

        public static byte MOVE_EAST_COMMAND { get; } = 0x01;

        public static byte MOVE_WEST_COMMAND { get; } = 0x02;

        public static byte MOVE_NORTH_COMMAND { get; } = 0x04;

        public static byte MOVE_SOUTH_COMMAND { get; } = 0x08;

        public static byte SET_POSITION_COMMAND { get; } = 0x23;

        public static byte SET_SYNC_COMMAND { get; } = 0x24;

        public static byte PARK_POSITION_COMMAND { get; } = 0x1e;

        public static byte STOP_MOTION_COMMAND { get; } = 0x1d;

        public static byte SET_SITE_LOCATION_COMMAND { get; } = 0x25;

        public static byte GET_SITE_LOCATION_COMMAND { get; } = 0x1F;

        public static byte SET_DATE_TIME_COMMAND { get; } = 0x26;

        public static int MESSAGE_FRAME_LENGTH { get; } = 13;

        public RightAscension RA { get; set; }

        public Declination DEC { get; set; }

        public Declination Latitude { get; set; }

        public Declination Longitude { get; set; }

        public byte CID
        {
            get
            {
                return MessageBuffer[4];
            }
        }

        public void UpdateValues()
        {
            float dec_ra = BitConverter.ToSingle(MessageBuffer, 5);
            float dec_decl = BitConverter.ToSingle(MessageBuffer, 9);

            print_buffer(MessageBuffer, 5, 4);

            print_buffer(MessageBuffer, 9, 4);

            RA = RightAscension.ConvertFrom(dec_ra);
            DEC = Declination.ConvertFrom(dec_decl);

            Latitude = Declination.ConvertFrom(dec_ra);
            Longitude = Declination.ConvertFrom(dec_decl);
        }

        public static void print_buffer(byte[] buffer, int offset,int length)
        {
            Debug.Write("Raw Buffer: ");
            for (int i = offset; i< offset + length; i++)
            {
                Debug.Write($"{String.Format("{0,2:X}", buffer[i])} ");
            }

            Debug.WriteLine("");
        }

        public static RawSerialMessage GetSetPointingLocationCommandMessage(RightAscension ra,Declination dec)
        {
            RawSerialMessage message = new RawSerialMessage() { RA = ra, DEC = dec };

            Buffer.BlockCopy(HEADER, 0, message.MessageBuffer, 0, HEADER.Length);

            message.MessageBuffer[4] = SET_POSITION_COMMAND;

            byte[] ra_bytes = BitConverter.GetBytes(ra.DecimalValue);
            byte[] dec_bytes = BitConverter.GetBytes(dec.DecimalValue);

            Buffer.BlockCopy(ra_bytes, 0, message.MessageBuffer, 5, ra_bytes.Length);
            Buffer.BlockCopy(dec_bytes, 0, message.MessageBuffer, 9, dec_bytes.Length);

            return message;
        }

        public static RawSerialMessage GetSyncPointingLocationCommandMessage(RightAscension ra, Declination dec)
        {
            RawSerialMessage message = new RawSerialMessage() { RA = ra, DEC = dec };

            Buffer.BlockCopy(HEADER, 0, message.MessageBuffer, 0, HEADER.Length);

            message.MessageBuffer[4] = SET_SYNC_COMMAND;

            byte[] ra_bytes = BitConverter.GetBytes(ra.DecimalValue);
            byte[] dec_bytes = BitConverter.GetBytes(dec.DecimalValue);

            Buffer.BlockCopy(ra_bytes, 0, message.MessageBuffer, 5, ra_bytes.Length);
            Buffer.BlockCopy(dec_bytes, 0, message.MessageBuffer, 9, dec_bytes.Length);

            return message;
        }

        public static RawSerialMessage GetDisconnectMessage()
        {
            RawSerialMessage message = new RawSerialMessage();

            Buffer.BlockCopy(HEADER, 0, message.MessageBuffer, 0, HEADER.Length);

            message.MessageBuffer[4] = DISCONNECT_COMMAND;

            return message;
        }

        public static RawSerialMessage GetParkCommandMessage()
        {
            RawSerialMessage message = new RawSerialMessage();

            Buffer.BlockCopy(HEADER, 0, message.MessageBuffer, 0, HEADER.Length);

            message.MessageBuffer[4] = PARK_POSITION_COMMAND;

            return message;
        }

        public static RawSerialMessage GetStopMotionCommandMessage()
        {
            RawSerialMessage message = new RawSerialMessage();

            Buffer.BlockCopy(HEADER, 0, message.MessageBuffer, 0, HEADER.Length);

            message.MessageBuffer[4] = STOP_MOTION_COMMAND;

            return message;
        }

        public static RawSerialMessage GetSetTimeDateCommandMessage(byte hours,byte minutes,byte seconds, byte day, byte month, short year)
        {
            RawSerialMessage message = new RawSerialMessage();

            Buffer.BlockCopy(HEADER, 0, message.MessageBuffer, 0, HEADER.Length);

            message.MessageBuffer[4] = SET_DATE_TIME_COMMAND;

            byte hiYear = (byte)(year / 100);
            byte loYear = (byte)(year % 100);
            //first date
            message.MessageBuffer[5] = hiYear;
            message.MessageBuffer[6] = loYear;
            message.MessageBuffer[7] = month;
            message.MessageBuffer[8] = day;

            //second time
            message.MessageBuffer[9] = hours;
            message.MessageBuffer[10] = minutes;
            message.MessageBuffer[11] = seconds;

            return message;
        }

        public static RawSerialMessage GetSetLocationCommandMessage(byte lat_deg, byte lat_min, byte lat_sec, byte long_deg, byte long_min, byte long_sec)
        {
            RawSerialMessage message = new RawSerialMessage();

            Buffer.BlockCopy(HEADER, 0, message.MessageBuffer, 0, HEADER.Length);

            message.MessageBuffer[4] = SET_SITE_LOCATION_COMMAND;

            Declination lat = new Declination() { Degrees = lat_deg, Minutes = lat_min, Seconds = lat_sec };
            Declination lon = new Declination() { Degrees = long_deg, Minutes = long_min, Seconds = long_sec };

            /*message.MessageBuffer[5] = lat_deg;
            message.MessageBuffer[6] = lat_min;
            message.MessageBuffer[7] = lat_sec;

            message.MessageBuffer[9] = long_deg;
            message.MessageBuffer[10] = long_min;
            message.MessageBuffer[11] = long_sec;*/

            //int latitude = 1;
            int longitude = 1;

            byte[] lat_bytes = BitConverter.GetBytes(lat.DecimalValue);
            byte[] long_bytes = BitConverter.GetBytes(lon.DecimalValue);

            Buffer.BlockCopy(lat_bytes, 0, message.MessageBuffer, 5, lat_bytes.Length);
            Buffer.BlockCopy(long_bytes, 0, message.MessageBuffer, 9, long_bytes.Length);

            return message;
        }

        public static RawSerialMessage GetGetLocationCommandMessage()
        {
            RawSerialMessage message = new RawSerialMessage();

            Buffer.BlockCopy(HEADER, 0, message.MessageBuffer, 0, HEADER.Length);

            message.MessageBuffer[4] = GET_SITE_LOCATION_COMMAND;

            return message;
        }

        public static RawSerialMessage GetGetGoDirectionCommandMessage(byte direction)
        {
            return GetFuzzCommandMessage(direction,0xc8,0,0,0,0xc8,0,0,0);
        }

        public static RawSerialMessage GetFuzzCommandMessage(byte command, byte b0=0x00,byte b1=0x00,byte b2=0x00,byte b3=0x00, byte b4=0x00, byte b5=0x00,byte b6=0x00, byte b7=0x00)
        {
            RawSerialMessage message = new RawSerialMessage();

            Buffer.BlockCopy(HEADER, 0, message.MessageBuffer, 0, HEADER.Length);

            message.MessageBuffer[4] = command;

            message.MessageBuffer[5] = b0;
            message.MessageBuffer[6] = b1;
            message.MessageBuffer[7] = b2;
            message.MessageBuffer[8] = b3;

            message.MessageBuffer[9] = b4;
            message.MessageBuffer[10] = b5;
            message.MessageBuffer[11] = b6;
            message.MessageBuffer[12] = b7;

            return message;
        }

        public override string ToString()
        {
            StringBuilder messageBuilder = new StringBuilder();

            if(MessageBuffer[4]==REPORT_POINTING_COORDINATES_RESPONSE)
            {
                messageBuilder.Append("Report -> ");
                messageBuilder.Append($"RA:{RA};DEC:{DEC}");
            }

            if(MessageBuffer[4] == SET_POSITION_COMMAND)
            {
                messageBuilder.Append("GOTO -> ");
                messageBuilder.Append($"RA:{RA};DEC:{DEC}");
            }

            if (MessageBuffer[4] == STOP_MOTION_COMMAND)
            {
                messageBuilder.Append("Stop Motion!");
            }

            if (MessageBuffer[4] == STOP_MOTION_COMMAND)
            {
                messageBuilder.Append("Stop Motion!");
            }

            if (MessageBuffer[4] == SET_DATE_TIME_COMMAND)
            {
                messageBuilder.Append("Setting Time -> ");
            }

            messageBuilder.Append("\nRaw Buffer: ");
            for (int i = 0; i < MessageBuffer.Length; i++)
            {
                messageBuilder.Append($"{String.Format("{0,2:X}", MessageBuffer[i])} ");
            }

            messageBuilder.Append("\n");


            return messageBuilder.ToString();
        }
    }
}
