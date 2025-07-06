import axios from 'axios';

const API_BASE_URL = 'http://localhost:8080';

export const createRoom = async () => {
    try {
        const response = await axios.post(`${API_BASE_URL}/Chat/CreateRoomId`);
        console.log('Room created successfully:', response);
        return response.data;
    } catch (error) {
        console.error('Error creating room:', error);
        throw error;
    }
}

export const ActiveRoom = async (roomId) => {
    try {
        const response = await axios.post(`${API_BASE_URL}/Chat/ActiveRoomId`, { roomId });
        console.log('Room activated successfully:', response);
        return response.data;
    } catch (error) {
        console.error('Error activating room:', error);
        throw error;
    }
}